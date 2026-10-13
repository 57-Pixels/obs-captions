# Rules for AI agents (and humans) working in this repo

This code is largely AI-written and lives outside OBS Studio (which does not
accept AI-generated contributions). It must stay easy for a human to read,
maintain, or rewrite by hand. Readability, small modules, tests and
documented decisions beat cleverness.

## Hard rules

1. **Never invent OBS APIs.** Before using any libobs or obs-frontend-api
   function, find it in the OBS headers fetched into `.deps/include`
   (`grep -rn <name> .deps/include`) and cite the header in a comment next
   to the call, e.g. `/* obs_output_output_caption_text2: libobs/obs.h */`.
   If you cannot find it, stop and ask.
2. **No speech recognition, model downloads or inference runtimes** unless
   the current task explicitly asks for them.
3. **No new third-party dependency** without recording name, version,
   license and reason in `docs/DEPENDENCIES.md`. Ask the maintainer before
   adding anything not GPL-2.0-compatible.
4. **Follow OBS conventions.** Linux kernel style, tabs for indentation and
   spaces for alignment, English names and comments, enforced by
   `.clang-format` (clang-format 19) and `.gersemirc` (gersemi). Commits:
   50/72, present tense, no trailing period, module prefix:
   `plugin:`, `core:`, `engine:`, `docs:`, `cmake:`.
5. **Small, reviewable commits.** One module per commit where possible.
6. **Ambiguous? Don't guess.** Add a specific question to
   `docs/OPEN_QUESTIONS.md` with your current assumption.
7. **Stop at checkpoints** and report what you did, what you verified, and
   what you are unsure about.

## Danger zones

- **OBS audio callback:** never block, allocate heavily, lock a contended
  mutex, log, or do I/O. Copy into the lock-free SPSC ring buffer and return.
- **References:** release every reference returned by `obs_*_get_*`
  (`obs_output_release`, `obs_source_release`, ...) in the same function.
- **Qt:** touch Qt objects only on the Qt UI thread. Other threads post to it.
- **Caption output:** all caption output goes through
  `plugin/src/caption-out.c`. No other file may call
  `obs_output_output_caption_text*`.
- **Stream start/stop:** libobs frees the caption queue when an output stops
  (`libobs/obs-output.c`). Re-arm caption state on every stream start.
- **core/ has zero OBS dependency.** No OBS headers, no OBS types, no clock
  reads (time is passed in).

## Process rules

- `core/` changes require golden-file tests written **first**
  (`core/tests/golden/`), then the implementation.
- No refactor may span `plugin/`, `core/` and `engine/` in one change.
- Template patches in `cmake/` are marked `obs-captions:`; keep them minimal
  and re-apply them after template updates (ADR 0006).

## Useful commands (Windows)

```
cmake --preset windows-x64
cmake --build --preset windows-x64
cmake -S core -B build_core
cmake --build build_core --config Debug
ctest --test-dir build_core -C Debug --output-on-failure
```
