# Next Tasks

Epic 1〜2 skeleton の次に着手する順序を固定する。

## 1. bypass-only output stage を実フレームに接続する

- `KfiFrameTap` を KWin final per-output frame 取得点へ接続する
- `KfiKwinHookAdapter` を実際の KWin private/internal event に差し替える
- `KfiGpuServices` の passthrough submission を実際の present path へ結線する
- final per-output composed frame の取得点を KWin 6 実コード上で確認する
- `Compositor::composite(RenderLoop *)` と `OutputFrame` 生成点を最初の候補として検証する
- `KfiKwinFrameBuilder` を使って `Compositor` 側 hook から metadata/payload/context を埋める
- `KfiKwinHookAdapter::preferred_frame_candidate()` を起点に frame hook 候補を固定する
- `KfiKwinFrameBuilder::is_complete()` を満たす field が KWin 実 hook から埋まることを確認する
- `KfiKwinFrameBuilder::missing_required_fields()` で不足 field を絞り込めるように、hook ごとの埋め元をコメントに残す
- field source plan helper と KWin 実ソースを突き合わせて、想定 source enum を TODO から確定候補へ更新する
- `KfiKwinHookCandidates::compositor_output_frame_ready()` の plan を起点に、`WaylandCompositor::composite(RenderLoop *)` で実際に取れる field を確認する
- `KfiKwinHookAdapter::assess_frame_candidate()` で incomplete field を開発時に即確認できるようにする
- `unresolved_fields` を 0 にできる候補だけを実差し替え対象に進める
- provenance context が `HookUnavailable` にならない形で final composed frame 境界を確認する
- private/internal hook 利用箇所へ明示コメントを入れる

## 2. Present feedback の受け渡しを作る

- per-output present completion 情報を拾う
- Rust 側に cadence/scheduler 用の入力 struct を追加する
- dropped / miss を metrics へ出せるようにする
- present feedback と last submitted frame の照合を実 KWin path でも維持する
- `OutputFrame::presented(...)` と `RenderLoop::framePresented(...)` のどちらで拾うかを backend ごとに比較する
- `KfiKwinPresentBuilder` を使って `OutputFrame` / `RenderLoop` hook から metadata/status/context を埋める
- `KfiKwinHookAdapter::preferred_present_candidate()` を起点に present hook 候補を固定する
- `KfiKwinPresentBuilder::is_complete()` を満たす feedback field が backend ごとに取れることを確認する
- `KfiKwinPresentBuilder::missing_required_fields()` で不足 field を backend ごとに記録する
- present 側の field source plan helper を backend ごとの実 callback と照合する
- `KfiKwinHookCandidates::output_frame_presented()` / `render_loop_frame_presented()` の plan を起点に backend ごとの差分を洗い出す
- `KfiKwinHookAdapter::assess_present_candidate()` で incomplete feedback field を開発時に即確認できるようにする
- `unresolved_fields` を backend ごとに削って、実際に採用する present callback を絞り込む
- unknown ではない present hook context を backend ごとに選び、ignore path を実 hook で踏まないようにする

## 3. bypass-only path をクラッシュしない最小経路にする

- C++ 側に GPU resource lifetime を閉じ込める
- Rust には opaque `GpuFrameHandle` だけを渡す
- deadline miss や hook 未接続時は即 bypass へ戻す

## 4. 設定と観測面を増やす

- enabled/disabled の読み込み口を KConfig に差し替える
- HUD/metrics 用の snapshot struct を追加する
- rate-limited logging を追加する
