# ADR 0006: Minimum OBS version is 30.2

Status: accepted (2026-10-12)

## Context

Twitch Enhanced Broadcasting and many current setups stream HEVC or AV1.
OBS writes text captions into HEVC SEI and AV1 metadata OBUs only since
30.2 (`libobs/obs-output.c`, `add_caption`, `hevc`/`av1` branches); older
versions carry captions only in H.264.

A plugin's effective minimum OBS version is the libobs it is compiled
against: OBS 32.x rejects modules built against a newer libobs
("compiled with newer libobs", `libobs/obs-module.c` since 32.0), while
30.2 and 31.x have no check at all, so a module built against 31.x could load into 30.2
and fail on missing symbols.

## Decision

Build against **OBS 30.2.3** (latest 30.2 patch) with its matching
obs-deps and Qt6 `2024-05-08` (`buildspec.json`).

The current obs-plugintemplate (3e7d7ac, Dec 2025) targets OBS 31+. Two
small, commented patches in `cmake/common/buildspec_common.cmake` make it
build 30.2.3:

- the inner OBS configure gets `-A x64` without `,version=` (OBS 30.2
  derives its dependency arch from `CMAKE_GENERATOR_PLATFORM` verbatim);
- `<deps>/cmake` is added to `CMAKE_PREFIX_PATH` (OBS 30.x installs package
  configs to `cmake/<name>` on Windows).

## Consequences

- Works on OBS 30.2 through current (32.x verified locally on 32.2.2).
- APIs added after 30.2 cannot be used. Any proposal to raise the minimum
  needs a new ADR.
- Template updates must re-apply the two patches (search for
  "obs-captions:" in `cmake/`).
