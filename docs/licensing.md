# Licensing Policy

Fluxma is dual-licensed as:

```text
MIT OR GPL-2.0-or-later
```

The repository keeps the full license texts in `LICENSES/MIT.txt` and
`LICENSES/GPL-2.0-or-later.txt`.

## Why dual-license?

The current skeleton has no third-party Rust dependencies and does not link to
Qt or KWin yet. The standalone source can therefore remain permissively
licensed.

Fluxma's intended final form is different: it is a KWin internal/native module
for Plasma 6 / KWin 6 / Wayland. Once code links against KWin internals, uses
KWin private headers, embeds into KWin, or is distributed as a KWin-native
module, that build must select the GPL-2.0-or-later side of this dual license.
The MIT side remains available for standalone Fluxma-owned source that is not
combined with GPL-covered KWin components.

This file is a project policy, not legal advice.

## Rules

- Keep new Fluxma-owned source files under `MIT OR GPL-2.0-or-later` unless a
  narrower license is explicitly justified in the same change.
- Do not copy KWin, Plasma, Qt, Mesa, or other upstream source into this
  repository unless the copied file's license is recorded and compatible with
  the target build.
- If a file is derived from GPL-covered KWin code, use the actual applicable
  upstream SPDX license for that file and document the origin in the commit.
- Do not add GPL-incompatible dependencies.
- Do not add AGPL, SSPL, BUSL, non-commercial, source-available, or custom
  restrictive dependencies without a dedicated license review.
- LGPL libraries are acceptable only when their linking and redistribution
  obligations are understood and documented.
- Qt GPL-only modules require the affected Fluxma distribution to select a GPL
  side. Avoid them unless there is no practical LGPL alternative.
- Keep KWin integration code separate from standalone Rust logic where possible
  so the licensing boundary stays visible.

## Dependency Classes

Allowed by default:

- MIT
- Apache-2.0
- BSD-2-Clause
- BSD-3-Clause
- ISC
- Zlib
- Unicode-DFS-2016 / Unicode-3.0
- MPL-2.0, if the file-level copyleft obligations are acceptable
- LGPL-2.1-or-later / LGPL-3.0-or-later, for dynamically linked system
  libraries with documented obligations

Requires review before merge:

- GPL-2.0-only
- GPL-2.0-or-later
- GPL-3.0-only
- GPL-3.0-or-later
- LGPL static linking
- Qt modules that are GPL-only for open-source users
- Any dependency with unclear, missing, or custom licensing

Rejected unless the project policy changes:

- AGPL
- SSPL
- BUSL
- Commons Clause
- Non-commercial or field-of-use restricted licenses
- Dependencies that would require DRM bypass, protected-content extraction, or
  behavior forbidden by `AGENTS.md`

## Practical Guidance

For standalone crates and test helpers, prefer permissive dependencies.

For KWin-facing C++ and plugin code, assume the distributed module selects
`GPL-2.0-or-later` unless proved otherwise. If a future package manager split is
added, the KWin-native package should advertise the GPL side of this dual
license.

For Qt/KCM work, record the exact Qt modules used. Most common Qt libraries are
available under LGPL for open-source use, but some Qt modules are GPL-only.

Before adding any dependency, update this policy or add a short note in the
commit explaining why the dependency is compatible.
