# Dependency License Inventory

This inventory tracks dependency license assumptions for Fluxma. Update it when
adding a crate, C++ library, shader/runtime component, KWin integration target,
or packaging dependency.

## Current State

| Component | Used for | License status | Notes |
| --- | --- | --- | --- |
| Fluxma-owned C++ code | Native skeleton and smoke tests | `MIT OR GPL-2.0-or-later` | No external C++ libraries linked yet. |
| `fluxma-rs-core` | Rust state/orchestration staticlib | `MIT OR GPL-2.0-or-later` | No Rust dependencies. |
| C++ standard library | Build/runtime support | System toolchain | Treat as system dependency. |
| Rust standard library | Build/runtime support | Rust toolchain | Treat as system dependency. |
| CMake / CTest | Build/test tooling | Tooling only | Not distributed as Fluxma runtime. |
| cocogitto | Commit and changelog tooling | Tooling only | Not distributed as Fluxma runtime. |

## Planned Dependencies

| Component | Expected use | License risk | Policy |
| --- | --- | --- | --- |
| KWin private/internal APIs | Native module integration | High | KWin-linked distributions must select the GPL-2.0-or-later side. |
| Qt / KDE Frameworks | KCM, config, and ABI boundary | Medium | Record exact modules before linking; avoid GPL-only Qt modules unless the target selects a GPL side. |
| Mesa / EGL / GL / Vulkan system libraries | GPU resource management | Medium | Prefer system dynamic linking and record actual libraries when introduced. |
| Optical flow / synthesis backend | Frame interpolation | Unknown | Must be reviewed before adding; no AGPL, non-commercial, or source-available-only dependencies. |

## Review Checklist

- Is the dependency runtime, build-only, test-only, or documentation-only?
- Is the license SPDX identifier known and compatible with the affected target?
- Does it introduce copyleft obligations that affect the whole KWin-native
  module?
- Does it require static linking, source redistribution, object-file relinking,
  or notice files?
- Does it touch protected content, DRM, capture, or recording behavior forbidden
  by project rules?
- Is the dependency still maintained and available from an upstream source?
