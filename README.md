# Fluxma

Fluxma は、**Plasma 6 / KWin 6 / Wayland** 向けの **KWin 内蔵型 post-display フレーム補間モジュール**です。

通常の QML デスクトップエフェクトではなく、**KWin の内部レンダーパイプライン**に差し込む **native module** として実装します。  
目的は、フルスクリーン動画視聴時に、出力直前の最終合成フレームに対して **2x 補間**を行い、見た目の滑らかさを向上させることです。

---

## このプロジェクトの特徴

- **Wayland 専用**
- **KWin 内部統合前提**
- **Rust + C++ の混成構成**
- **Rust**: 状態機械、cadence 推定、補間判定、スケジューリング、メトリクス
- **C++**: Qt/KWin ABI、GPU リソース管理、レンダーフック、最終提示
- **protected content は必ず passthrough-only**
- 不安定時は補間より **bypass を優先**

---

## MVP の対象範囲

### 対応
- Plasma 6 / KWin 6
- Wayland セッション
- 単一出力
- フルスクリーン動画視聴
- 2x 補間
- cursor passthrough
- subtitle / UI 保護の初期対応
- protected content の即時バイパス

### 非対応
- X11
- マルチモニタ同期補間
- HDR / 色管理の完全対応
- 3x / 4x / 5x 補間
- AI backend（RIFE など）の初期対応
- 録画、保存、書き出し
- DRM 回避や protected path 介入

---

## 文書

- 実装ルール: `AGENTS.md`
- 詳細設計: `docs/architecture.md`
- 実装順序: `docs/tasks.md`

---

## 実装方針

このプロジェクトは、まず **KWin fork 内蔵前提**で進めます。  
公開された安定プラグイン API に寄せることよりも、**MVP を正しく成立させること**を優先します。

最初の到達点は以下です。

1. mixed build の骨格を作る
2. Rust/C++ ブリッジを最小限で通す
3. KWin 側の統合ポイント候補を特定する
4. bypass-only の output stage を成立させる
5. HUD とメトリクスを出す

---

## 現時点での重要な前提

- これは **QML desktop effect ではない**
- これは **media player plugin でもない**
- 対象は **最終合成済みの output frame**
- Rust は **GPU resource lifetime を持たない**
- protected content では **補間を試みない**

---

## 開発メモ

詳細な設計と制約は `AGENTS.md` と `docs/architecture.md` を参照してください。  
Codex / Claude Code を使う場合も、まずその 2 つを読む前提で進めてください。
