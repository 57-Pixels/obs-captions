# Open questions

Ambiguities are written here instead of guessed. Each item has a specific
question, the current assumption, and who/what can answer it. Answered
items move to "Resolved" with the decision, so the reasoning is kept.

## Open

### Caption output (verify on Twitch in roadmap phase 2)

4. **Clearing the screen.** Does `obs_output_output_caption_text2` with an
   empty string (or a single space) clear the 608 display on Twitch desktop
   and mobile? libcaption's `caption_frame_from_text("")` produces an empty
   frame, but player behaviour is unverified.
5. **Pop-on rows.** libcaption writes rows from row 0 (top). Do Twitch
   players show 2-row pop-on captions near the bottom, and is 28 columns
   readable on phones?
11. **608 glyphs on Twitch.** Do Twitch players render 608 extended
    characters (accented letters, the music note) correctly? And should
    smart quotes and dashes keep becoming ASCII (current, per the brief), or
    use 608's own curly-quote and em-dash glyphs? Decide after testing.

### Caption behaviour (core/)

6. **ASR during TTS.** While a TTS line is active, ASR hypotheses are dropped
   (counted as `tts_active`). Should they instead be shown after the TTS line
   ends?
7. **Hallucination blocklist languages.** v1 blocklist is English only. Which
   other Latin-script languages need lists, if the filter is ever enabled?
8. **"Short" in the hallucination rule** (matters only when the filter is
   enabled). The filter only fires when the *entire* sentence is a known
   artifact phrase, e.g. just "Thank you." and nothing else. The longest
   artifact phrases (subtitle credits) are 6 words, so the word limit is 8.
   Keep, or make it smaller?
14. **TTS lines and the word list.** A TTS app sends the exact text it
    speaks, so TTS lines skip the word-replacement list and the repeat
    filter; they still get the profanity mask and are converted to the 608
    character set. Should the word list also apply to TTS lines (e.g. to fix
    a name the TTS app spells differently)?
17. **Voice-activity signal.** When the real engine exists (phase 4), it
    should report per hypothesis whether voice was detected, so the artifact
    filter can drop invented text without any timing delay. Field name and
    format to be added to PROTOCOL.md then.

## Resolved

1. **80 vs 120 columns** (2026-10-13): keep the template's clang-format
   config, `ColumnLimit: 120`, matching upstream OBS.
2. **Compiler** (2026-10-13): Visual Studio 2026 is the main local toolchain
   (`windows-x64-vs2026` preset). CI keeps the template's VS 2022 runner
   until the template or GitHub runners move; binaries are compatible.
3. **Project name** (2026-10-13): renamed to **simple-local-captions**
   ("Simple Local Captions") to avoid the OBS trademark. The internal C++
   namespace is `slc`.
9. **Isolation rule cost** (2026-10-13): not acceptable. The silence-artifact
   filter is **off by default**; it stays available for Whisper-style
   backends. See 17 for the planned replacement signal.
10. **Artifacts in a row** (2026-10-13): moot for the default engine, which
    is not Whisper. If the filter is enabled, a held candidate is still only
    released by non-blocklisted speech.
12. **Exact re-sends** (2026-10-13): yes. A final with the same words that
    overlaps the previous one in time is dropped at the 2nd copy and does not
    count toward the repeat run. Speech repeated later is unaffected.
13. **Speaker markers** (2026-10-13): keep ">> " only when both speakers are
    known; real markers need diarization (a later engine feature).
15. **Long monologues** (2026-10-13): old segments are trimmed in batches of
    16 (when more than 64 are kept), so text re-wraps rarely.
16. **Maximum rows** (2026-10-13): 4 is enough; good timing matters more.
