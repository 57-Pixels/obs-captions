# Open questions

Ambiguities are written here instead of guessed. Each item has a specific
question, the current assumption, and who/what can answer it.

## Build and conventions

1. **80 vs 120 columns.** The brief says 80 columns enforced by the
   template's clang-format config, but that config (current OBS style) sets
   `ColumnLimit: 120`. Current assumption: keep the template config (120) so
   the CI check matches upstream. Should we lower it to 80?
2. **Local compiler.** VS 2026 cannot select MSVC 14.44 through the CMake
   "Visual Studio 18 2026" generator (CMake 4.3 rejects every toolset/version
   spec tried). Local builds use MSVC 14.51; CI (VS 2022, 14.4x) is the
   reference. Acceptable, or should contributors install VS 2022 Build Tools?
3. **Project name.** "obs-captions" uses the OBS name. Is that acceptable
   under OBS trademark guidance, or should we pick another name before the
   first release?

## Caption output (verify in roadmap phase 2)

4. **Clearing the screen.** Does `obs_output_output_caption_text2` with an
   empty string (or a single space) clear the 608 display on Twitch desktop
   and mobile? libcaption's `caption_frame_from_text("")` produces an empty
   frame, but player behaviour is unverified.
5. **Pop-on rows.** libcaption writes rows from row 0 (top). Do Twitch
   players show 2-row pop-on captions near the bottom, and is 28 columns
   readable on phones?

## Caption behaviour (core/)

6. **ASR during TTS.** While a TTS line is active, ASR hypotheses are dropped
   (counted as `tts_active`). Should they instead be shown after the TTS line
   ends?
7. **Hallucination blocklist languages.** v1 blocklist is English only. Which
   other Latin-script languages need lists for v1?
