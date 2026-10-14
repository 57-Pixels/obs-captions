# Dependencies

Every third-party component, with version, license and reason. Anything not
GPL-2.0-compatible needs the maintainer's approval before it is added.

## Shipped or linked

| Name | Version | License | Where | Why |
|------|---------|---------|-------|-----|
| OBS Studio (libobs, obs-frontend-api) | 30.2.3 | GPL-2.0-or-later | `buildspec.json`, fetched to `.deps/` | Plugin host API; minimum version per ADR 0006 |
| obs-deps (prebuilt) | 2024-05-08 | various, GPL-compatible (as shipped with OBS 30.2.3) | `buildspec.json` | Build dependencies OBS 30.2.3 was released with |
| Qt 6 (obs-deps build) | 2024-05-08 (Qt 6.6.3) | LGPL-3.0 | `buildspec.json` | Dock UI; same Qt that OBS ships, linked dynamically |
| sherpa-onnx (C API, prebuilt) | 1.13.8 | Apache-2.0 | `engine/cmake/sherpa-onnx.cmake` (SHA-256 pinned), header in `engine/third_party/sherpa-onnx/` | Streaming speech recognition in the engine process only (ADR 0007); approved 2026-10-13 |
| ONNX Runtime (bundled with sherpa-onnx) | as shipped in sherpa-onnx 1.13.8 | MIT | `bin/64bit/engine/` | Neural network inference for sherpa-onnx; engine process only |
| Speech model: sherpa-onnx-streaming-zipformer-en-2023-06-21 | 2023-06-21 | Apache-2.0 (model card; icefall libri-giga stateless7 streaming) | not in repo; `%LOCALAPPDATA%\simple-local-captions\models\` | Default English model (ADR 0007) |
| libcaption (inside libobs) | as in OBS 30.2.3 | MIT | not linked directly | Does UTF-8 to CEA-608 encoding; core's charset mirrors its table |

## Build and test only (not shipped)

| Name | Version | License | Where | Why |
|------|---------|---------|-------|-----|
| obs-plugintemplate | 3e7d7ac (2025-12-09) | GPL-2.0 | repo scaffolding | Official build system, presets, CI |
| doctest | 2.5.3 | MIT | `core/third_party/doctest/` | Single-header test framework for core/ |
| clang-format | 19.1.x | Apache-2.0 WITH LLVM-exception | CI (`obsproject/tools`) | C/C++ formatting check |
| gersemi | >= 0.12 (0.19.3 used locally) | MPL-2.0 | CI | CMake formatting check |
| Python 3 (standard library only) | 3.8+ | PSF-2.0 | `core/formatter/gen-608-tables.py` | Optional: regenerates the 608 tables from OBS's libcaption; not needed to build |
