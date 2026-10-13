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
8. **"Short" in the hallucination rule.** The brief lists subtitle-credit
   phrases ("Subtitles by the Amara.org community", 6 words) but also says
   the rule applies only to short hypotheses. Assumption: a candidate must
   match a blocklist entry as the *whole* utterance, and `max_words`
   defaults to 8 so credit phrases qualify. Is 8 right, or should credits
   be a separate always-on list?
9. **Cost of the isolation rule.** A genuine "thank you" said into silence
   (nothing within 2 s before or after) is never shown, and a blocklisted
   phrase followed by speech appears up to 2 s late. Acceptable trade-off?
10. **Artifacts in a row.** A held candidate is released only by
    non-blocklisted speech; a second blocklisted phrase does not vouch for it
    (Whisper often repeats "Thank you." on silence). Correct?
11. **Smart quotes on screen.** CEA-608 has curly quote and em-dash glyphs,
    but per the brief they are converted to ASCII. Keep that, or use the 608
    glyphs? Also unverified: whether Twitch players render 608 extended
    characters (accents, music note) correctly. Check in phase 2.
12. **Repeat threshold meaning.** "Repeated 3+ times" is implemented as: the
    first two consecutive (near-)duplicate finals are shown, the 3rd and later
    are dropped. Inside one utterance a phrase repeated 3+ times is cut to 2.
    Should exact duplicates (likely engine re-sends) be dropped already at
    the 2nd occurrence?
13. **Speaker markers.** ">> " is added only when both the previous and the
    new speaker are known and differ. Unknown speakers (no diarization yet)
    never get markers. Should a TTS line always be marked, even after
    unlabelled ASR text?
14. **TTS text filtering.** TTS lines skip word replacement, repeat and
    hallucination filters (they are exact), but still get the profanity mask
    and transliteration. Right?
15. **Long monologues.** The scheduler keeps at most 64 segments; dropping the
    oldest can, rarely, change where an on-screen line wraps. Acceptable, or
    trim only at paragraph boundaries?
16. **Maximum rows.** Rows are capped at 4 (608 pop-on practice; 4 x 32
    characters plus newlines already approaches libobs' 128-byte limit).
    Does anyone need more?
