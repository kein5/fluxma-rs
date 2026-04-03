# Fluxma KWin Integration Notes

このメモは、Epic 2 時点で確定できていない KWin internal/native hook 候補を整理するためのもの。
ここでは候補を列挙し、無理に決め打ちしない。

## 前提

- QML effect は使わない
- native module として KWin 内部へ接続する
- 処理対象は final per-output composed frame
- protected content 区間では passthrough-only
- Rust は orchestration/state のみを担当し、GPU resource lifetime は C++ 側で持つ

## Hook 候補

### 1. Final per-output frame の取得点

候補:
- per-output render pass 完了直後で、cursor や HUD を後段で差し戻せる位置
- KWin scene/output backend で最終 render target が確定した直後
- present 直前で damage や present feedback と結びつけやすい位置

要求:
- output ごとに独立して扱えること
- final composed frame の GPU handle を C++ 側で保持できること
- synthetic frame を deadline までに差し込めない場合に元フレームへ即戻せること

TODO:
- 実際の KWin 6 ソースで `Output`, `RenderLoop`, `Compositor`, backend 関連の private hook 候補を確認する
- private/internal hook を使う箇所には実装時に明示コメントを付ける

2026-04-03 時点で KWin master の一次情報から確認できた候補:
- `src/compositor.cpp` の `Compositor::composite(RenderLoop *)`
  - `renderLoop->prepareNewFrame()` の後で `OutputFrame` を生成する
  - `frame->setContentType(...)` で active fullscreen item の content type を設定する
  - `output->present(toUpdate, frame)` へ流す直前まで、final per-output composed frame の制御が集中している
- `src/core/renderbackend.h` / `src/core/renderbackend.cpp` の `OutputFrame`
  - `targetPageflipTime()`, `refreshDuration()`, `predictedRenderTime()` を持つ
  - `presented(...)` で backend feedback と `RenderLoop` をつなぐ
- `src/backends/drm/drm_pipeline.cpp` の `DrmPipeline::present(...)`
  - `OutputFrame` を atomic/legacy present commit に渡す境界
- `src/backends/wayland/wayland_output.cpp` の `WaylandOutput::present(...)`
  - `wp_presentation_feedback` と `wl_surface_frame` を束ねて `OutputFrame` と関連付ける境界

メモ:
- layered rendering の流れでは primary/cursor をまとめて atomic に扱う変更が進んでいる
- single-output 限定の overlay plane 利用が進んでおり、MVP の単一出力制約と相性が良い
- cursor は `composite()` で generic overlay として更新する流れがあり、cursor を補間しない方針と整合しやすい
- `Compositor::composite(RenderLoop *)` は cursor layer を別 layer として扱っており、primary content と cursor passthrough の分離点としても有力

### 2. Presentation feedback の取得点

候補:
- per-output present completion を受けられる箇所
- refresh interval と actual presented timestamp を観測できる箇所

用途:
- 将来の cadence estimator
- scheduler の deadline 補正
- dropped synthetic の計測

TODO:
- render loop / present feedback の受け取り面を KWin 6 側で確認する

メモ:
- `src/core/renderbackend.cpp` の `OutputFrame::presented(...)`
  - `RenderLoopPrivate::notifyFrameCompleted(timestamp, renderTime, mode, this)` を呼ぶ
  - `PresentationFeedback::presented(refreshCycleDuration, timestamp, mode)` も同時に通知する
- `src/core/renderloop.cpp` の `RenderLoopPrivate::notifyFrameCompleted(...)`
  - pending frame 数、render journal、vblank timestamp を更新したあと `framePresented(RenderLoop *, timestamp, mode)` を emit する
- `src/backends/wayland/wayland_output.cpp` の `WaylandOutput::framePresented(...)`
  - `wp_presentation` の timestamp/refresh を `OutputFrame::presented(...)` に返す
- `src/backends/drm/drm_commit.cpp` の `DrmAtomicCommit::pageFlipped(...)` と `DrmLegacyCommit::pageFlipped(...)`
  - page flip 完了後に `frame->presented(timestamp, m_mode)` を呼ぶ

Fluxma の scheduler / cadence 用 feedback 候補:
- backend 固有 timestamp を拾いたいなら `OutputFrame::presented(...)` の手前
- backend 差を均したいなら `RenderLoop::framePresented(...)` の手前または直後
- bypass-only MVP では、まず `frame_id + timestamp + refresh interval + presentation mode` を保持できれば十分

### 3. Cursor / overlay 後段再合成位置

候補:
- 補間対象外の cursor を最後に重ねられる pass
- HUD debug overlay を最終パスで描ける位置

要求:
- cursor は補間しない
- HUD は最終パスで描く

TODO:
- cursor plane 合成との境界を確認する
- overlay 再合成が難しい backend では MVP では bypass を優先する

KWin source 上の観察:
- `src/compositor.cpp` では cursor が `OutputLayerType::CursorOnly` として特別扱いされる
- hardware cursor path が壊れた場合は primary のみへ fallback する分岐がある
- Fluxma は cursor を補間しない前提なので、まずは primary layer 相当だけを対象にし、cursor layer には介入しないのが安全

## 現時点の実装判断

- `KfiPluginRoot` と `KfiOutputController` は native module 前提の C++ skeleton とする
- `KfiOutputController` は将来の per-output hook 着地点として維持する
- `KfiFrameTap` は skeleton のみ追加し、現時点では入力された `FrameDescriptor` をそのまま pipeline に流す
- `KfiKwinHookAdapter` を追加し、KWin 由来の frame/present event と internal controller の境界を明示した
- `KfiOutputPolicy` を追加し、single-output 制約と unsupported-input 判定を adapter から分離した
- `KfiPresentFeedbackTap` を追加し、present feedback 側も frame tap と同じ粒度で差し替え可能にした
- adapter の present 側は `PresentCompletedMetadata` と `PresentCompletedStatus` の split でも受けられるようにした
- adapter には `KwinFrameHookContext` / `KwinPresentHookContext` も追加し、`Compositor` 境界なのか `OutputFrame` / `RenderLoop` 境界なのかを API 上で明示できるようにした
- `KfiKwinFrameBuilder` / `KfiKwinPresentBuilder` を追加し、各 KWin 境界で埋める metadata/status/context の組を helper として固定した
- `KwinCompositorFrameInputs` / `KwinPresentFeedbackInputs` を追加し、実 hook 実装時にどの field を埋めるべきかを input struct として固定した
- builder から `KwinResolvedFrameHook` / `KwinResolvedPresentHook` をまとめて作れるようにし、実 hook 側は `inputs -> bundle -> adapter` の形で流せるようにした
- builder には `is_complete()` を追加し、`frame_id/timestamp/size/gpu_handle` や `frame_id/presented_timestamp/refresh_interval` が欠けた入力は sentinel event に畳むようにした
- builder には `missing_required_fields()` も追加し、実 hook 実装時にどの必須 field が未取得なのかを bitmask で追えるようにした
- missing field bitmask は `describe(...)` で安定した名前へ変換できるようにし、実 hook 実装時の debug log / TODO 切り分けに使えるようにした
- builder には field source plan helper も追加し、`Compositor` / `OutputFrame` / `RenderLoop` / backend present path のどこから各 field を埋める想定かを enum で保持するようにした
- `KfiKwinHookCandidates` を追加し、現時点の一次情報ベースの候補を `source_file + symbol + note + required_fields` としてコードで固定した
- `KfiOutputPolicy` は provenance context も見るようにし、MVP で未対応な frame hook 境界は `HookUnavailable` bypass、unknown present hook 境界は ignore に倒す
- present feedback では `frame_id/presented_timestamp_ns/refresh_interval_ns` が欠けた入力も ignore に倒し、欠損 metadata を Rust metrics に流さない
- adapter 層では width/height/gpu handle の最低限 validation を行い、未対応入力は `unsupported-output` bypass に倒す
- MVP の単一出力制約に合わせ、target output 以外の event は adapter 層で無視または `unsupported-output` bypass に倒す
- `FrameDescriptor` には KWin `ContentType` 相当の `content_type` を持たせ、`PresentFeedback` には `presentation_mode` を持たせた
- `FrameDescriptor` には `target_presentation_timestamp_ns` と `predicted_render_time_ns` も持たせ、`OutputFrame` timing hint を保持できるようにした
- HUD / metrics でも `content_type` と `presentation_mode` を見えるようにし、将来の classifier / scheduler 入力を先に固定した
- adapter 入口は `FinalComposedFrameMetadata` と `FinalComposedFramePayload` の split でも受けられるようにし、KWin 実 hook 側で metadata と payload の取得点が分かれても差し替えやすくした
- `PresentFeedback` の受け口は追加し、Rust core が feedback count / refresh interval を保持する
- present feedback の `frame_id` は metrics に保持し、last submission と食い違う場合は mismatch log を出す
- HUD は `KfiHudRenderer` で文字列生成のみを行い、state / bypass reason / protected flag / refresh interval を表示できる
- synthetic frame 生成、shader dispatch、KConfig/KCM 本実装はまだ入れない

## KWin 6.3.6 source cross-check

以下は 2026-04-03 時点で確認した一次情報ベースの候補であり、まだ決め打ちではない。

- `src/compositor_wayland.cpp::WaylandCompositor::composite(RenderLoop *)`
  - `OutputFrame` の生成は `frame = std::make_shared<OutputFrame>(...)`
  - `frame->setContentType(...)`、`frame->setPresentationMode(...)`、`m_backend->present(output, frame)` が同じ関数内にある
  - MVP の frame tap 候補として最有力
- `src/core/renderbackend.cpp::OutputFrame::presented(...)`
  - present completion を `OutputFrame` 単位で受ける候補
- `src/core/renderloop.cpp::RenderLoopPrivate::notifyFrameCompleted(...)`
  - `OutputFrame` 側で不足した backend 差分を吸収する fallback 候補

backend present handoff は `m_backend->present(output, frame)` の先にある各 backend 実装であり、
frame handle や page flip completion の埋め元候補ではあるが、MVP では final composed semantics が
確定するまで `HookUnavailable` 扱いを維持する。
