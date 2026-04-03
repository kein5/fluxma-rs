# Fluxma

Fluxma は Plasma 6 / KWin 6 / Wayland 向けの KWin 内蔵型 post-display フレーム補間モジュールである。

このリポジトリの現段階は、Epic 1〜2 相当の skeleton に限定している。

- QML effect は使わない
- KWin internal/native module 前提で進める
- Rust は orchestration/state を担当する
- C++ は KWin/Qt ABI と GPU resource lifetime を担当する
- protected content は passthrough-only

## 現在入っているもの

- mixed build skeleton (`CMake` + `Cargo`)
- C++/Rust 最小ブリッジ
- `KfiPluginRoot` / `KfiOutputController` の骨格
- bypass-only path 用の基本型
- output ごとの state / present feedback / HUD text skeleton
- KWin integration notes

## 現在まだ入っていないもの

- optical flow
- synthesis shader
- subtitle heuristic の詳細実装
- KCM の本実装

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
cargo test --manifest-path src/rust/Cargo.toml
```
