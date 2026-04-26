# Fluxma

Fluxma は Plasma 6 / KWin 6 / Wayland 向けの KWin 内蔵型 post-display フレーム補間モジュールである。

このリポジトリの現段階は、Epic 1〜6 の skeleton / placeholder 実装まで進めている。

- QML effect は使わない
- KWin internal/native module 前提で進める
- Rust は orchestration/state を担当する
- C++ は KWin/Qt ABI と GPU resource lifetime を担当する
- protected content は passthrough-only

## 現在入っているもの

- mixed build skeleton (`CMake` + `Cargo`)
- C++/Rust 最小ブリッジ
- `KfiPluginRoot` / `KfiOutputController` の骨格
- bypass-only path と protected passthrough-only
- output ごとの state / cadence / governor / scheduler skeleton
- present feedback / HUD / metrics / rate-limited logging
- fake synthetic plan / artifact / placeholder present queue
- KWin integration notes / native bridge placeholder

## 現在まだ入っていないもの

- optical flow
- synthesis shader
- real synthetic present path
- subtitle heuristic の詳細実装
- KCM の本実装

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
cargo test --manifest-path src/rust/Cargo.toml
```

## License

Fluxma is dual-licensed as `MIT OR GPL-2.0-or-later`.

Standalone source may be used under MIT. Builds that link against or derive
from GPL-covered KWin internals must use the GPL-2.0-or-later side. See
[docs/licensing.md](docs/licensing.md) before adding Qt/KWin integration code
or new dependencies.

## Release notes

This repository uses cocogitto for Conventional Commits and changelog generation.

```bash
cog install-hook commit-msg
cog check --from-latest-tag
cog bump --auto
```

Commits must use one of the standard Conventional Commit types accepted by
cocogitto: `build`, `chore`, `ci`, `docs`, `feat`, `fix`, `perf`, `refactor`,
`revert`, `style`, or `test`.

`v0.1.0` is the baseline tag for the pre-cocogitto history. Generate future
release entries with `cog bump` from commits after that tag rather than
re-rendering the old non-conventional history.
