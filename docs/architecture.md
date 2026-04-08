# Fluxma 詳細設計書

## 1. 目的

Fluxma は、Wayland 上の KWin に **post-display frame interpolation** を組み込むためのモジュールである。  
目的は、**フルスクリーン動画視聴時の最終合成フレーム**に対して 2x の補間を行い、表示を滑らかにすることにある。

本プロジェクトは以下を対象とする。

- Plasma 6 / KWin 6
- Wayland セッション
- KWin 内部レンダーパイプライン
- 単一出力
- フルスクリーン動画視聴

本プロジェクトは以下を対象としない。

- QML の desktop effect
- アプリ内部の動画デコードパイプライン
- protected content の抽出や回避
- 録画、保存、再配布

---

## 2. 設計の基本方針

### 2.1 基本原則

Fluxma は、**アプリ内部の映像ストリームではなく、KWin が output 用に最終合成したフレーム**を対象に処理する。

したがって設計上の主眼は次の 4 つである。

- output frame の取得
- 補間可否の判定
- 中間フレームの生成
- 安全な提示スケジューリング

### 2.2 protected content の扱い

protected content に対しては、**補間を行わず passthrough-only** とする。  
この点は設計上の必須条件であり、例外を設けない。

### 2.3 実装方式

本機能は **KWin の公開 QML effect API ではなく、KWin 内部 hook を用いる native module** として実装する。

---

## 3. MVP の範囲

### 3.1 対応
- Wayland のみ
- 単一出力
- フルスクリーン動画視聴
- 2x 補間のみ
- cursor passthrough
- subtitle / UI 保護の初期対応
- protected content の即時バイパス

### 3.2 非対応
- X11
- multi-monitor
- HDR
- 高度な color management
- 3x 以上の補間
- AI backend（RIFE など）
- 録画 / 保存
- browser 個別最適化

---

## 4. 全体アーキテクチャ

## 4.1 レイヤ構成

Fluxma は次の 2 層で構成する。

### C++ / Qt / KWin shim 層
責務:
- KWin internal hook
- output frame の受け取り
- KWin event から internal frame/present struct への変換
- GPU resource lifetime 管理
- shader dispatch
- HUD 描画
- KConfig / KCM
- 最終提示

### Rust core 層
責務:
- state machine
- cadence estimator
- classifier
- governor
- scheduler
- flow / synth orchestration
- metrics

### 境界方針
- Rust は KWin object を直接持たない
- Rust は GPU resource lifetime を持たない
- GPU handle は opaque handle として Rust に渡す
- 実際の GPU リソース管理は C++ 側で行う

---

## 5. レンダーパイプライン上の位置

概念上の処理順序は次の通り。

1. KWin が scene graph を更新
2. output 向けに通常の合成を実行
3. final per-output frame を得る
4. Fluxma が補間可否を判定
5. 必要なら中間フレームを生成
6. cursor / 一部 overlay を後段で再合成
7. output へ提示

重要なのは、**cursor を補間しない**ことと、**最終提示直前に補間を差し込む**ことである。

---

## 6. 出力ごとの構成

各 output ごとに独立した controller を持つ。

主な構成要素:

- `KfiOutputController`
- `KfiKwinHookAdapter`
- `KfiOutputPolicy`
- `KfiBypassPipeline`
- `KfiFrameTap`
- `KfiPresentFeedbackTap`
- `KfiGpuServices`
- `RustOutputCore`
- `KfiHudRenderer`

MVP では単一出力のみ対象とするが、内部的には output ごとの controller で持つ設計にする。

---

## 7. 主要データ構造

## 7.1 FrameDescriptor

Rust に渡す output frame の記述子。

含める情報:
- frame id
- timestamp
- target presentation timestamp
- predicted render time
- width / height
- pixel format
- color space / range
- content type
- protected flag
- damage ratio
- cursor visible / position / velocity

現在の C++ adapter では、KWin 実境界に寄せるために
`FinalComposedFrameMetadata` と `FinalComposedFramePayload` を分けて受け、
それを `FrameDescriptor` に束ねる。
present feedback 側も `PresentCompletedMetadata` と `PresentCompletedStatus` に分けて受け、
`PresentFeedback` に束ねる。
さらに `KwinFrameHookContext` / `KwinPresentHookContext` で、
どの private/internal 境界から来た情報かを明示できるようにしている。
実 hook 実装時は `KwinCompositorFrameInputs` / `KwinPresentFeedbackInputs` を埋めて、
builder 経由で adapter へ流す前提にしている。
また builder には `is_complete()` を持たせ、不完全な hook 入力からは
sentinel event しか生成しないことで、欠損 metadata をそのまま adapter に流さない。
さらに field source plan helper を持たせ、frame/present の各必須値を
`Compositor` / `OutputFrame` / `RenderLoop` / backend present path のどこから
埋める想定かを型として保持している。
`KfiKwinHookAdapter` 自体も preferred candidate/readiness helper を公開し、
実 hook 差し替え時の入口を adapter 境界に寄せている。
checklist と readiness summary も adapter から直接引けるため、
bring-up 時の診断は candidate helper を直接辿らず adapter API で閉じる。
さらに `KfiKwinNativeBridge` を placeholder-only で追加し、
native/internal hook の将来差し替え先を plugin root 配下に固定した。
bridge は preferred candidate, checklist, readiness summary を集約して返すため、
plugin root から native bring-up 状態を一段で観測できる。
install 経路も stub 化してあり、placeholder-only の間は deferred reason を
明示的に返して実差し替え前の責務境界を固定している。
frame hook と present hook の install stub は分離してあり、将来の bring-up を
別々に進められるようにしている。
各 install stub は target 名だけでなく `source_file` と `symbol` も返すため、
実際にどの KWin entry point を触る前提なのかを bridge 側で固定できる。
さらに `checklist_hint` も返すので、最初に潰す確認事項を install 導線から直接見られる。
`checklist_hint_secondary` も持たせてあり、最初の 2 手まで install report から追える。
加えて `checklist_all` も返すので、候補ごとの bring-up 手順全体を bridge 側から直接確認できる。
`installer_entry` も返すため、このリポジトリ内でどの空シグネチャを差し替え先にするかまで固定できる。
さらに `deferred_reason` enum を持たせ、placeholder-only / version gate / backend gate を区別できるようにした。
install stub は `KwinNativeInstallContext` も受けられるようにし、
KWin version と backend の gate を placeholder 分岐とは別に注入できる。
これにより native hook bring-up 前でも、「未接続だから保留」と
「version/backend 条件が満たせず保留」を report 上で分離できる。
`KwinNativeInstallContext` を与えれば、実 hook 差し替え前でも KWin version gate と backend gate のどちらで
止めているのかを install report 上で切り替えて観測できる。
加えて `context_summary` も report に保持し、gate 判定時の KWin version / backend 名と
support 状態を install summary 1 本で確認できるようにしている。
同じ gate 判定は `assess_install_gate(KwinNativeInstallContext)` にも切り出し、
将来の real installer が report 生成と同じ条件分岐を共有できるようにしている。
さらに `preflight_frame_install()` / `preflight_present_install()` を用意し、
install 実行前でも `gate + target + source/symbol + checklist` を 1 本の report として確認できる。
`preflight_install()` はその frame/present 両方を束ね、combined summary と同じ粒度で
native bridge の install 前診断を扱えるようにする。
combined preflight/install report 自体にも frame/present 単位の helper を持たせ、
combined report を unpack せずに deferred reason や deferred state を追えるようにしている。
`KfiPluginRoot::observe_native_bridge(...)` は `bringup + preflight + install stub` を
1 つの observation report に束ね、plugin root から native bridge の状態を直接観測できる。
`KfiPluginRoot::observe_native_bridge_bringup(...)` は hook completeness だけを追う用途、
`observe_native_bridge_install(...)` は install gate と deferred state だけを追う用途、
`observe_native_bridge(...)` は両者をまとめて残す用途として使い分ける。
`observe_native_bridge_bringup(...)` は hook 入力の completeness/readiness だけを見たい場合に使い、
`observe_native_bridge_install(...)` は install 前後の gate 一致だけを見たい場合に使う。
observation report には deferred reason と version/backend blocker flag の helper もあり、
plugin root 利用側は nested report を直接 unpack せずに frame/present ごとの gate 状態を確認できる。

## 7.2 GpuFrameHandle

GPU resource を Rust が直接所有しないための opaque handle。

- backend kind
- handle id

## 7.3 PresentFeedback

実際の提示タイミングを scheduler に返すための情報。

- presented timestamp
- refresh interval
- presentation mode
- present success/failure
- dropped synthetic info

現段階の skeleton では、controller 側に small history を持ち、
直近の submitted frame id 群と照合して mismatch を検出する。

## 7.4 MetricsSnapshot

HUD / logging 用の実行状態。

- state
- source fps
- output hz
- current decision
- bypass reason
- protected flag
- content type
- presentation mode
- target presentation timestamp
- predicted render time
- flow time
- synth time
- deadline miss count

現状の skeleton では、`MetricsSnapshot` に加えて次も保持する。

- cadence status
- cadence millihz
- classifier allow/interpolate flag
- governor mode
- scheduler mode
- state transition count

これらは Rust state core が更新し、C++ 側は HUD / log で読むだけに留める。

---

## 8. 状態機械

output ごとの状態は次の enum で表現する。

- `Disabled`
- `Bypass`
- `Warmup`
- `Active2x`
- `Degraded`
- `ProtectedBypass`
- `Faulted`

### 遷移方針
- 初期状態は `Bypass`
- cadence 安定後に `Warmup`
- 安定継続で `Active2x`
- deadline miss が増えたら `Degraded`
- protected flag で `ProtectedBypass`
- GPU fault や import 失敗で `Faulted`

### 基本原則
失敗や不確実性がある場合は、上位状態へ無理に進まず **Bypass へ戻す**。

adapter 層で未対応 output / 未対応 frame event を検出した場合も、
controller や GPU path を無理に進めず bypass を返す。

---

## 9. cadence estimator

cadence estimator は、presentation feedback から source fps の安定性を推定する。

主な役割:
- 24 / 25 / 30 / 50 / 60fps 付近へのスナップ
- jitter の算出
- 安定 / 不安定の判定

安定していない場合は補間を行わない。

現状の skeleton では、まず frame timestamp の差分から cadence を見積もる。
直近 small window の delta を nominal cadence
(`24/25/30/50/60fps`) に snap し、jitter が小さい場合だけ stable とみなす。
presentation feedback は governor/deadline 側へ返し、次段の scheduler 判断に使う。

---

## 10. classifier

classifier は、この区間で補間を行うべきか判定する。

入力:
- cadence stability
- damage ratio
- scene cut score
- cursor velocity
- deadline pressure
- protected flag

出力:
- `Bypass(reason)`
- `Interpolate2x`

主な bypass 理由:
- disabled
- protected content
- cadence unstable
- scene cut
- cursor fast motion
- deadline pressure
- gpu fault

現状の skeleton では、classifier は次だけを見る。

- cadence stable
- `ContentType::Video`
- 一定以上の damage ratio
- 速すぎない cursor velocity

scene cut はまだ未実装で、後続 task に残す。
subtitle band は C++ 側 `KfiProtectionPlanner` で placeholder heuristic を持ち、
現在は HUD / runtime observation に出すだけで synth weighting にはまだ接続していない。

---

## 11. governor

governor は、負荷や deadline miss に応じて品質を段階的に落とし、必要なら bypass に戻す。

MVP では次のような単純な段階で十分。

- `QualityHigh`
- `QualityMedium`
- `QualityLow`
- `Bypass`

初期実装では、品質低下よりまず bypass を優先してもよい。

現状の skeleton では、deadline miss / dropped synthetic の累積だけで
`Bypass -> QualityMedium -> QualityLow` 相当の mode を返す。
まだ real synth を持たないため、mode は HUD/metrics に出すだけで
GPU 品質段階には接続していない。

---

## 12. scheduler

scheduler は、real frame と synthetic frame の提示順序を決める。

MVP は 2x のみなので、基本形は次の通り。

- 30fps -> 60Hz: `A -> AB -> B`
- 24fps -> 48Hz: `A -> AB -> B`
- 60fps -> 120Hz: `A -> AB -> B`

ルール:
- synthetic frame が deadline に間に合わなければ捨てる
- 連続 miss で degrade または bypass
- 不安定なら real frame のみ出す

現状の skeleton では、Rust scheduler は
`PassthroughOnly / WarmupHold / Synthetic2x` の mode 判定を返し、
state machine はこの mode を見て `Bypass / Warmup / Active2x / Degraded` を更新する。
C++ 側ではこれに対応して、次の placeholder synthetic chain を持つ。

- `KfiSyntheticScheduler`
- `KfiFakeSynthGenerator`
- `KfiSyntheticPresentQueue`

この chain は `SyntheticFramePlan -> SyntheticFrameArtifact -> SyntheticPresentSubmission`
を順に生成し、real GPU synth や real present queue をまだ使わずに
synthetic path の timing と drop contract だけを先に固定する。
`SyntheticPresentSubmission` は queue 成否、drop、placeholder-only を持ち、
HUD / logging / output runtime observation で可視化される。
この段階でも decision には `interpolation_armed` を保持し、
C++ 側へ「まだ placeholder path だが、synthetic slot を要求している」ことを伝える。

---

## 13. flow / synth

### 13.1 MVP 方針
MVP の最初から本物の optical flow を作らない。  
まずは以下の順序で進める。

1. bypass-only
2. fake synth frame
3. low-resolution optical flow
4. midframe synthesis

現状の skeleton では、本物の synth frame の前段として
`KfiSyntheticScheduler` が placeholder synthetic slot を計画し、
`KfiFakeSynthGenerator` が placeholder artifact を生成し、
`KfiSyntheticPresentQueue` が placeholder submission へ変換する。
この段階では GPU frame をまだ生成せず、`interpolation_armed` と
`refresh_interval_ns` / `predicted_render_time_ns` から
`target_present_timestamp_ns` と `deadline_timestamp_ns` を決め、
deadline miss 時は artifact/submission の両方で即 drop する。
protected content や bypass state では synthetic plan 自体を arm しない。
ただし `KfiSyntheticPresentQueue` は現段階では stateless projection であり、
real queue/backpressure をまだ表現しない。`synthetic_queued` は placeholder chain の
観測値であって、real present queue ready を意味しない。

### 13.2 初期実装
初期版では、軽量な low-resolution flow または block matching を想定する。  
RIFE のような AI backend は後回しにする。

### 13.3 現在の placeholder GPU skeleton
real flow の前段として、C++ 側には次の placeholder GPU skeleton を追加している。

- `KfiTexturePool`
- `KfiLumaPyramidBuilder`
- `KfiFlowInputsBuilder`
- `KfiConfidenceMapBuilder`

`KfiTexturePool` は fixed-capacity の placeholder texture lease を管理し、
overflow や invalid descriptor では fail-safe に acquire を拒否する。
`KfiLumaPyramidBuilder` は source descriptor から downsample level の寸法列だけを先に固定し、
各 level 用の placeholder lease を pool から取る。
`KfiFlowInputsBuilder` は prev/curr 2 frame 分の source texture と luma pyramid を束ね、
real motion field の前段で必要になる GPU input bundle を placeholder として固定する。
`KfiConfidenceMapBuilder` は usable な flow input bundle から level ごとの placeholder
confidence bias を生成し、confidence map 導入前の形状と解像度契約だけを先に固定する。
現段階では real luma extraction や real compute dispatch は行わず、
pool 枯渇時や invalid descriptor では fail-safe に build を止め、lease を解放して
non-usable bundle を返す。
この bundle は `is_usable()` が真のときだけ次段へ渡す想定であり、
placeholder slice の範囲では real flow dispatch の成立を意味しない。
この skeleton の目的は、Epic 7 で必要になる GPU lifetime と pyramid 形状を
C++ 側に固定することであり、Rust に GPU resource ownership を渡さない点は維持する。

### 13.4 placeholder midframe synthesis
Epic 8 の入口として、`KfiMidframeSynthesizer` を placeholder-only で追加している。
これは usable な `FlowInputBundle` と `ConfidenceMapBuildResult`、および
synthetic frame id / target present timestamp を受け取り、real shader dispatch をまだ行わずに
midframe synthesis request/result の契約だけを先に固定する。
この段階で返すのは metadata のみだが、`KfiSyntheticPresentQueue` へ
placeholder synthesis result を渡す seam までは接続している。
real synthetic frame resource や real GPU present queue 返却はまだ行わない。

---

## 14. cursor / subtitle / overlay 保護

### cursor
cursor は補間しない。  
補間後に現在位置で再合成する。

現状の skeleton では `KfiProtectionPlanner` が
`cursor_visible && cursor_protection` を見て
`cursor_passthrough / cursor_recomposite` を立てる。
まだ real post-synth recomposite には未接続で、HUD / observation のみ。

### subtitle
画面下部の字幕帯は heuristic で検出し、その領域では current frame 側を強く優先する。

現状の skeleton では `ContentType::Video` かつ non-protected の frame に対して、
画面下部 18% を placeholder subtitle band として返す。
placeholder `MidframeSynthesisResult` には
`prefer_current_in_subtitle_band` を通しているが、
さらに placeholder `SyntheticPresentSubmission` にも伝搬しているが、
まだ real synth weighting / mask 適用には未接続で、HUD / observation / synth metadata のみ。

### overlay
一時的な OSD や UI は、可能なら補間対象から除外する。  
ただし MVP では完全な semantic 分離を目指さない。

現状の skeleton では KWin hook context の `overlay_promoted` を
`FrameDescriptor` と `ProtectionPlan` に通し、
`transient_overlay_passthrough` として HUD / runtime observation に出す。
まだ real mask / layer 分離には未接続。

---

## 15. protected content ポリシー

protected content の区間では、次の動作のみ許可する。

- 元フレームをそのまま提示
- 補間を無効化
- HUD / log に debug 情報を出す

次は禁止する。

- 補間の試行
- ストリーム抽出
- 保存・書き出し
- DRM 回避前提の設計

現状の skeleton では `MetricsSnapshot::is_protected_bypass()` と
`OutputRuntimeObservationReport::synthetic_suppressed_by_protection()` を持ち、
さらに `SyntheticPresentSubmission::is_protected_suppressed()` も持つため、
typed observation と synthetic submission の両方から protected passthrough-only を確認できる。

---

## 16. スレッド方針

### render thread
- KWin 側のレンダリング
- frame tap
- final submit
- cursor / overlay 再合成

### worker / GPU 側
- luma pyramid
- flow
- synth

### control 側
- cadence
- classifier
- governor
- scheduler
- metrics

### KCM bridge 層
現状の skeleton では `src/kcm/fluxma_kcm_bridge.*` を追加し、
`ModuleConfig` と `observe_output_runtime()` を plain C++ snapshot に変換している。
runtime 側は `state/bypass/protected/passthrough/synthetic/cadence/hud_text` に加えて
`frame_tap_count/present_feedback_count/deadline_miss_count/dropped_synthetic_count`
もそのまま KCM bridge へ渡す。
native bridge 側も install 診断だけを plain snapshot として取り出せる。
ここでは `frame/present deferred reason` と `version/backend gate` のみを持たせ、
actual installer や Qt/KCM page はまだ実装しない。
加えて bringup 側も `frame/present complete` と `unresolved` を plain snapshot として
取り出せる。KWin private/internal hook の未確定箇所は summary のまま露出し、
KCM bridge 側で意味を決め打ちしない。
さらに combined diagnostics snapshot では bringup/install をまとめて参照できるが、
それでも KCM bridge 側は `gate match` や `blocker` の転写に留める。
これは将来の KCM UI 層を薄く保つための bridge で、まだ actual Qt/KCM page には未接続。

原則:
- render hot path に重い判断や QObject-heavy 処理を置かない
- Rust は orchestration を担う
- GPU resource の所有は C++ 側

---

## 17. 最初の到達点

最初の実装ターゲットは次の通り。

1. mixed build を通す
2. Rust/C++ bridge の最小往復を通す
3. KWin integration point の候補を特定する
4. bypass-only output stage を成立させる
5. HUD と metrics を出す

この時点では、まだ optical flow や real synthesis を入れない。

---

## 18. 成功条件

MVP 初期段階の成功条件は次の通り。

- KWin がクラッシュせず起動する
- module の on/off が可能
- bypass-only path が動く
- HUD に state と bypass reason が出る
- cadence estimator の骨格ができる
- protected content は常に bypass される

---

## 19. 実装メモ

実装時は以下を守ること。

- private/internal hook 使用箇所には必ずコメントを付ける
- 未確定な KWin 統合点は TODO として残す
- 先に動く skeleton を作る
- 後から詳細化する
- 不明点がある場合は拡張より bypass を選ぶ
