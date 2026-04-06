# Fluxma 開発ルール

## プロジェクトの位置づけ

Fluxma は、**Plasma 6 / KWin 6 / Wayland** 向けの **KWin 内蔵型 post-display フレーム補間モジュール**である。

これは以下ではない。

- QML の KWin desktop effect
- 一般的なウィンドウエフェクト
- メディアプレーヤープラグイン
- 録画/書き出しツール
- DRM 回避ツール

処理対象は、**KWin が output に対して最終合成したフレーム**である。

---

## 最重要ルール

1. **QML effect として実装しないこと**
2. **KWin internal/native module 前提で実装すること**
3. **Rust は主要ロジック、C++ は Qt/KWin ABI と GPU リソース管理を担当すること**
4. **Rust に GPU resource lifetime を持たせないこと**
5. **render hot path に QObject-heavy な構造を入れないこと**
6. **不安定な場合は補間より bypass を優先すること**
7. **protected content では必ず passthrough-only にすること**
8. **protected content に対して独自処理を試みないこと**

---

## Rust と C++ の責務分担

### Rust の責務
- 状態機械
- cadence 推定
- classifier
- governor
- scheduler
- メトリクス管理
- flow / synth の orchestration
- bypass 理由の判定

### C++ の責務
- KWin / Qt / QObject ABI
- KWin internal hook
- GPU リソース生成/破棄
- texture / render target / sync の管理
- shader dispatch
- 最終提示
- HUD 描画
- KConfig / KCM

---

## 実装順序ルール

必ず次の順序で進めること。

1. 起動しても壊れない skeleton を作る
2. mixed build を通す
3. Rust/C++ の最小ブリッジを作る
4. KWin 側の output hook 候補を特定する
5. bypass-only path を成立させる
6. HUD と metrics を入れる
7. cadence estimator を実装する
8. scheduler / state machine を固める
9. fake synth frame を入れる
10. その後で初めて real flow / synthesis に進む

---

## 今回の MVP でやること

- Plasma 6 / KWin 6 / Wayland のみ
- 単一出力のみ
- フルスクリーン動画のみを主対象とする
- 2x 補間のみ
- cursor passthrough
- subtitle / UI 保護の初期対応
- protected content の即時バイパス
- deadline miss 時の degrade / bypass

---

## 今回の MVP でやらないこと

- X11 対応
- マルチモニタ同期補間
- HDR 対応
- 高度な色管理
- 3x / 4x / 5x 補間
- AI backend（RIFE など）
- 録画 / 保存 / 書き出し
- ブラウザ個別対応
- browser UI の semantic parsing
- protected content の補間保証
- DRM 回避
- protected path 介入

---

## protected content に関する厳守事項

protected content であると判定された区間では、唯一の正しい挙動は **passthrough-only** である。

許可される動作:
- 元フレームをそのまま提示する
- 補間を無効化する
- HUD / log に debug 情報を出す

禁止される動作:
- protected な映像を抜き出そうとする
- protected な区間で補間を試みる
- 録画/保存に流す
- DRM 回避を前提にした設計にする

---

## レンダリング上のルール

1. 処理対象は **final per-output composed frame**
2. cursor は補間しない
3. HUD は最終パスで描画する
4. synthetic frame が deadline に間に合わなければ捨てる
5. 失敗時はクラッシュより bypass を優先する
6. bypass 理由は enum で管理する
7. private/internal hook を使う箇所には必ず明示コメントを入れる

---

## コード品質ルール

- 小さく明示的な struct を優先する
- 曖昧な抽象化を急がない
- hot path と UI/config path を分離する
- state transition は debug log 可能にする
- deadline 関連の判断は metrics に出す
- 未確定な KWin hook は決め打ちせず TODO として残す

---

## テストルール

最低限テストすべき対象:

- cadence estimator
- classifier
- governor
- scheduler
- bypass-only path
- protected-content simulation
- seek / pause / resume
- fast cursor movement
- 24fps / 30fps fullscreen playback

---

## 自走運用ルール

- ユーザーが停止や方針変更を指示しない限り、自走で次の実装スライスへ進むこと
- 各スライスは build/test が通る最小単位で閉じ、必ず commit すること
- 各スライス完了後は `auditor` subagent と `tester` subagent に review させ、その指摘を次スライスの入力にすること
- 原則として進捗の逐次報告は省き、ブロッカー、失敗、判断が必要な分岐、または節目の要約だけを返すこと
- この運用ルールは永続とし、以後のターンでも継続適用すること

---

## 不明点がある場合の方針

不明な場合は次を優先すること。

1. より単純な実装
2. より安全な実装
3. 補間より bypass
4. 将来拡張より現在の正しさ
5. TODO とコメントで明示
