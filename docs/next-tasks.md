# Next Tasks

Epic 1〜2 skeleton の次に着手する順序を固定する。

## 1. bypass-only output stage を実フレームに接続する

- `KfiFrameTap` を KWin final per-output frame 取得点へ接続する
- `KfiKwinHookAdapter` を実際の KWin private/internal event に差し替える
- `KfiGpuServices` の passthrough submission を実際の present path へ結線する
- final per-output composed frame の取得点を KWin 6 実コード上で確認する
- `Compositor::composite(RenderLoop *)` と `OutputFrame` 生成点を最初の候補として検証する
- `KfiKwinFrameBuilder` を使って `Compositor` 側 hook から metadata/payload/context を埋める
- private/internal hook 利用箇所へ明示コメントを入れる

## 2. Present feedback の受け渡しを作る

- per-output present completion 情報を拾う
- Rust 側に cadence/scheduler 用の入力 struct を追加する
- dropped / miss を metrics へ出せるようにする
- present feedback と last submitted frame の照合を実 KWin path でも維持する
- `OutputFrame::presented(...)` と `RenderLoop::framePresented(...)` のどちらで拾うかを backend ごとに比較する
- `KfiKwinPresentBuilder` を使って `OutputFrame` / `RenderLoop` hook から metadata/status/context を埋める

## 3. bypass-only path をクラッシュしない最小経路にする

- C++ 側に GPU resource lifetime を閉じ込める
- Rust には opaque `GpuFrameHandle` だけを渡す
- deadline miss や hook 未接続時は即 bypass へ戻す

## 4. 設定と観測面を増やす

- enabled/disabled の読み込み口を KConfig に差し替える
- HUD/metrics 用の snapshot struct を追加する
- rate-limited logging を追加する
