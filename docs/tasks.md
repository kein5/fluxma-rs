# Fluxma 実装タスク

この文書は、Fluxma の実装順序を固定するためのタスクリストである。  
MVP を安全に前進させるため、**下からではなく上から順に**進めること。

---

## Epic 1: プロジェクト骨格

### Task 1.1
リポジトリ構造を整理する。

- `src/cpp/`
- `src/rust/`
- `src/shaders/`
- `src/kcm/`
- `tests/`
- `scripts/`

### Task 1.2
mixed build の骨格を作る。

- `CMakeLists.txt`
- `Cargo.toml`
- `build.rs`

### Task 1.3
C++ から Rust を呼ぶ最小ブリッジを作る。

- `RustOutputCore` の no-op 実装
- 初期化と破棄の往復確認

### Task 1.4
設定の最小骨格を作る。

- enabled/disabled の feature flag
- config 読み込みの土台

### 完了条件
- build が通る
- module skeleton が存在する
- C++ と Rust の最小往復ができる

---

## Epic 2: KWin 統合ポイントの特定

### Task 2.1
KWin 側で必要な統合ポイント候補を調査する。

対象:
- final per-output frame 取得点
- presentation feedback 取得点
- cursor / overlay 後段再合成の可能位置

### Task 2.2
統合候補をコードコメントまたは設計メモに残す。

### Task 2.3
`KfiPluginRoot` と `KfiOutputController` の骨格を作る。

### 完了条件
- KWin internal hook 候補が整理されている
- output ごとの controller 骨格がある

---

## Epic 3: bypass-only output stage

### Task 3.1
`KfiFrameTap` を実装し、final output frame のメタデータを拾えるようにする。

### Task 3.2
`FrameDescriptor` と `GpuFrameHandle` を定義する。

### Task 3.3
補間なしで元フレームをそのまま提示する bypass-only path を作る。

### Task 3.4
private/internal hook 使用箇所に明示コメントを入れる。

### 完了条件
- final output frame を pipeline に流せる
- 補間なし passthrough が成立する
- クラッシュしない

---

## Epic 4: HUD と metrics

### Task 4.1
HUD を表示できるようにする。

表示項目:
- current state
- bypass reason
- protected flag
- output refresh rate

### Task 4.2
基本メトリクスを追加する。

- frame tap count
- present feedback count
- deadline miss count
- dropped synthetic count

### Task 4.3
rate-limited logging を追加する。

### 完了条件
- bypass-only 状態でも HUD が見える
- 状態がログ/HUDから分かる

---

## Epic 5: Rust state core

### Task 5.1
状態機械を実装する。

- Disabled
- Bypass
- Warmup
- Active2x
- Degraded
- ProtectedBypass
- Faulted

### Task 5.2
cadence estimator を実装する。

### Task 5.3
classifier を実装する。

### Task 5.4
governor を実装する。

### Task 5.5
scheduler の骨格を実装する。

### Task 5.6
Rust unit test を追加する。

### 完了条件
- state / cadence / scheduler の基本ロジックが Rust 側にある
- テストがある

---

## Epic 6: fake synthetic frame

現状:
- `KfiSyntheticScheduler` で placeholder synthetic plan を生成済み
- `KfiFakeSynthGenerator` で placeholder artifact を生成済み
- `KfiSyntheticPresentQueue` で placeholder submission へ変換済み
- 30fps -> 60Hz 相当の smoke test で armed / generated / queued / dropped を確認済み
- まだ real GPU synth / real present queue には接続していない

### Task 6.1
本物の flow の前に fake synth frame を作る。

例:
- 単純な blend
- テスト用の中間フレーム

### Task 6.2
scheduler と synthetic present slot を接続する。

### Task 6.3
30fps -> 60Hz のテストシナリオで挙動を確認する。

### 完了条件
- synthetic frame の挿入経路が存在する
- timing に応じて drop できる

補足:
- 現在の `synthetic_queued` は stateless placeholder 観測値であり、real present queue ready を意味しない

---

## Epic 7: low-resolution flow

現状:
- `KfiTexturePool` で placeholder texture lease を管理済み
- `KfiLumaPyramidBuilder` で placeholder pyramid level を構築済み
- `KfiFlowInputsBuilder` で prev/curr の placeholder flow input bundle を構築済み
- `KfiConfidenceMapBuilder` で placeholder confidence map level を構築済み
- overflow 時は fail-safe に acquire/build を止め、non-usable/truncated bundle を返す
- まだ motion field / block matching には進んでいない

### Task 7.1
texture pool を実装する。

### Task 7.2
luma pyramid を実装する。

### Task 7.3
low-resolution optical flow または block matching を実装する。

### Task 7.4
confidence map の初期版を実装する。

### 完了条件
- prev/curr から motion field 相当を生成できる

補足:
- 現段階の texture/pyramid は C++ 側の GPU lifetime 骨格であり、Rust へ所有権は渡さない

---

## Epic 8: midframe synthesis

現状:
- `KfiMidframeSynthesizer` で placeholder midframe synthesis request/result を構築済み
- usable な flow input bundle と confidence map が揃った場合のみ placeholder synthesis を返す
- `KfiSyntheticPresentQueue` へ placeholder synthesis result を流す seam まで追加済み
- まだ real shader dispatch / real synthetic frame resource には進んでいない

### Task 8.1
2x 用の midframe synthesis を実装する。

### Task 8.2
Rust orchestration と C++ GPU service をつなぐ。

### Task 8.3
synthetic frame を present queue に返す。

### 完了条件
- 実際の中間フレーム生成が動く
- 24fps / 30fps テストで確認できる

---

## Epic 9: cursor / subtitle / overlay 保護

現状:
- `KfiProtectionPlanner` で cursor passthrough / recomposite flag を placeholder 判定済み
- `ContentType::Video` 向けの placeholder subtitle band heuristic を追加済み
- placeholder `MidframeSynthesisResult` に subtitle-band current-priority flag を伝搬済み
- placeholder `SyntheticPresentSubmission` にも protection metadata を伝搬済み
- KWin hook context の `overlay_promoted` を placeholder overlay passthrough flag に伝搬済み
- protected content では subtitle band を無効化し、cursor plan は descriptive flag のみ返す
- `OutputRuntimeSample` / HUD / runtime observation まで protection plan を可視化済み
- まだ real post-synth cursor recomposite / subtitle weighting / overlay exclusion には未接続

### Task 9.1
cursor を補間パスから外す。

### Task 9.2
cursor を後段再合成する。

### Task 9.3
subtitle band の heuristic を入れる。

### Task 9.4
字幕帯では current frame 側を優先する。

### Task 9.5
可能なら transient overlay を補間対象から外す。

### 完了条件
- cursor が二重化しにくい
- 字幕付近の破綻が減る

---

## Epic 10: protected content handling

### Task 10.1
protected flag の受け渡しを実装する。

### Task 10.2
protected 区間で強制 bypass を入れる。

### Task 10.3
HUD / log で protected bypass を可視化する。

### Task 10.4
protected で補間を試みないことをテストする。

現状:
- Rust core で protected frame は `ProtectedBypass` + passthrough-only 済み
- C++ runtime observation に typed helper を追加し、protected bypass と synthetic suppression を確認可能
- `SyntheticPresentSubmission` 自体にも protected/suppressed metadata を保持済み
- fake synth / runtime observation の smoke test で protected 区間では synthetic が arm/queue されないことを固定済み

### 完了条件
- protected 区間は常に passthrough-only

---

## Epic 11: KCM と仕上げ

現状:
- `src/kcm/fluxma_kcm_bridge.*` で KCM 向け settings/runtime snapshot bridge を追加済み
- `enabled/mode/show_hud/subtitle_protection/cursor_protection/logging` を plain C++ settings snapshot として取得可能
- `state/bypass/protected/passthrough/synthetic/cadence/hud_text` を live runtime snapshot として取得可能
- `frame_tap_count/present_feedback_count/deadline_miss_count/dropped_synthetic_count` も live runtime snapshot として取得可能
- native bridge install diagnostics も `frame/present deferred reason` と `version/backend gate` だけ plain C++ snapshot として取得可能
- native bridge bringup diagnostics も `frame/present complete` と `unresolved` を plain C++ snapshot として取得可能
- native bridge combined diagnostics も `bringup complete/unresolved/gate match/blocker/deferred reason` を plain C++ snapshot として取得可能
- まだ Qt/QML/actual KCM page には未接続

### Task 11.1
KCM の基本ページを作る。

設定項目:
- enabled
- mode
- show HUD
- subtitle protection
- cursor protection

### Task 11.2
live metrics を出せるようにする。

### Task 11.3
seek / pause / resume を含めた動作確認を行う。

### Task 11.4
output hotplug や fault recovery を確認する。

### 完了条件
- 基本設定が UI から変えられる
- failure 時も安全に bypass できる

---

## 実装上の補助ルール

- 不明な KWin hook は決め打ちしない
- まず bypass-only を通す
- protected content で補間を試みない
- AI backend は後回し
- 実装順序を飛ばさない
