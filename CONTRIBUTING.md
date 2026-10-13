# Contributing

Thanks for helping streamers caption their streams. Read
[AGENTS.md](AGENTS.md) first: its rules apply to humans too.

## Build (Windows x64)

Requirements: Visual Studio 2022 with "Desktop development with C++"
(or VS 2026, see below), CMake 3.28+, Git.

```
cmake --preset windows-x64
cmake --build --preset windows-x64
```

The first configure downloads OBS 30.2.3 sources, obs-deps and Qt6 into
`.deps/` and builds libobs (several minutes).

**VS 2026 only:** the repository presets use the VS 2022 generator, like CI.
With only VS 2026 installed (it bundles CMake 4.x), create a git-ignored
`CMakeUserPresets.json`:

```json
{
  "version": 8,
  "configurePresets": [{
    "name": "windows-x64-vs2026", "inherits": ["windows-x64"],
    "generator": "Visual Studio 18 2026", "architecture": "x64",
    "cacheVariables": {"CMAKE_SYSTEM_VERSION": "10.0.26100.0"}
  }],
  "buildPresets": [{"name": "windows-x64-vs2026",
    "configurePreset": "windows-x64-vs2026", "configuration": "RelWithDebInfo"}]
}
```

Use plain `x64` and set `CMAKE_SYSTEM_VERSION` to an installed Windows SDK.
This builds with MSVC 14.5x; CI's VS 2022 build is the reference.

## Test

```
cmake -S core -B build_core
cmake --build build_core --config Debug
ctest --test-dir build_core -C Debug --output-on-failure
```

To try the plugin in OBS, install it into a scratch folder and copy the
resulting `obs-captions/` folder into `C:\ProgramData\obs-studio\plugins\`:

```
cmake --install build_x64 --config RelWithDebInfo --prefix dist
```

## Format

CI checks C/C++ with clang-format **19** and CMake with gersemi. Other
clang-format versions can produce different output.

```
clang-format -i plugin/src/*.c plugin/src/*.h
gersemi -i CMakeLists.txt plugin/CMakeLists.txt
```

## Commits and pull requests

- 50/72 messages, present tense, no trailing period, module prefix
  (`plugin:`, `core:`, `engine:`, `docs:`, `cmake:`).
- One module per commit where possible; no change spans plugin/, core/ and
  engine/ at once.
- If an AI tool wrote a substantial part of a change, say so in the pull
  request description.
- By contributing you agree your work is licensed GPL-2.0-or-later.
