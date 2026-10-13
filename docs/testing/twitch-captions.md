# Manual test: captions on Twitch (roadmap phase 2)

Native captions can only be checked on a real stream. This takes about
10 minutes and answers open questions 4, 5 and 11.

## Setup

1. Install the plugin: copy the `simple-local-captions` folder from a build
   (`cmake --install build_x64 --config RelWithDebInfo --prefix dist`, or
   the Windows artifact of a CI run) into `C:\ProgramData\obs-studio\plugins\`.
2. Start OBS. Open **Docks > Captions** if the dock is not visible.
3. Note your settings: **Settings > Output** encoder (x264, NVENC H.264,
   HEVC or AV1) and whether Enhanced Broadcasting is on.
4. Have a second device (phone with the Twitch app) and a browser open on
   your channel. Captions arrive with the normal stream delay.

Tip: stream at a quiet time, or to a test channel; Twitch's bandwidth test
mode hides the stream from viewers, so captions cannot be checked there.

## Steps and what to record

| # | Do | Check on desktop browser | Check on phone app |
|---|----|--------------------------|--------------------|
| 1 | Press **Send test caption** before streaming | Dock says "You are not streaming" | - |
| 2 | Start streaming, wait ~15 s, press **Send test caption** | A **CC** button appears in the player; turn it on. Two lines: "Simple Local Captions test" / "Captions are working!" | Player settings > Captions on; same two lines |
| 3 | Where is the text? | Bottom or top of video? Readable size? | Same; is 28 characters per row comfortable? |
| 4 | Press **Send test caption** again | "Accents: café niño über" / "Symbols: ♪ © ½ ¿Sí?" Are all accents and symbols right? | Same |
| 5 | Press **Clear captions** | Does the caption disappear? | Same |
| 6 | Send a test caption and wait 30 s without pressing anything | Does it stay until replaced, or vanish by itself? | Same |
| 7 | Stop and restart the stream, send a test caption | Still works after restart | - |

## Report back

- Encoder and Enhanced Broadcasting on/off
- Results for steps 2-7 (screenshots help)
- Anything in the OBS log (Help > Log Files > Show Current Log) mentioning
  `simple-local-captions`
