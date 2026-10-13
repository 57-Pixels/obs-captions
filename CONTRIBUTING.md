# Contributing

Thanks for helping streamers caption their streams. Read
[AGENTS.md](AGENTS.md) first: its rules apply to humans too.

## Build (Windows x64)

Requirements: Visual Studio 2026 with "Desktop development with C++"
(it bundles CMake 4.x), Git.

```
cmake --preset windows-x64-vs2026
cmake --build --preset windows-x64-vs2026
```

The first configure downloads OBS 30.2.3 sources, obs-deps and Qt6 into
`.deps/` and builds libobs (several minutes).

CI builds with the template's Visual Studio 2022 preset (`windows-x64`,
`windows-ci-x64`) on GitHub's `windows-2022` runner; that preset also works
locally if you have VS 2022. Both produce compatible binaries.

## Test

```
cmake -S core -B build_core
cmake --build build_core --config Debug
ctest --test-dir build_core -C Debug --output-on-failure
```

To try the plugin in OBS, install it into a scratch folder and copy the
resulting `simple-local-captions/` folder into `C:\ProgramData\obs-studio\plugins\`:

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
