# Manual test: captions (roadmap phase 2)

Three levels, from quickest to most complete. Levels 1 and 2 need no
stream and no Twitch account.

## Setup

1. Install the plugin: copy the `simple-local-captions` folder from a build
   (`cmake --install build_x64 --config RelWithDebInfo --prefix dist`, or
   the Windows artifact of a CI run) into `C:\ProgramData\obs-studio\plugins\`.
2. Start OBS. Open **Docks > Captions** if the dock is not visible.

## Level 1: preview only (10 seconds)

| Do | Expect |
|----|--------|
| Press **Send test caption** | The black preview box shows "Simple Local Captions test" / "Captions are working!"; status says "Shown in the preview only" |
| Press it again | "Accents: café niño über" / "Symbols: ♪ © ½ ¿Sí?" |
| Press **Clear captions** | The preview box empties |

This checks the dock and the caption text, not the video encoding.

## Level 2: local recording (2 minutes, real CEA-608)

OBS puts the same captions into recordings as into the stream, inside the
video itself. This checks the real caption encoding without going live.

1. Press **Start Recording** in OBS.
2. Press **Send test caption**, wait 5 s, press it again, wait 5 s, press
   **Clear captions**, wait 5 s. The status should say "Added to your
   recording".
3. Stop recording and open the file in **VLC**.
4. In VLC: **Subtitle > Sub Track > Closed captions 1** (name may vary).
5. Record: do both captions appear at the right times, with accents and
   symbols correct? Does Clear remove the caption? Does a caption stay up on
   its own until replaced?

Note your recording encoder and format (**Settings > Output**); captions
are supported for H.264, HEVC and AV1 video.

## Level 3: Twitch (optional, needs a live stream)

1. Start streaming. Press **Send test caption** (status: "Sent to your
   stream").
2. Desktop browser: a **CC** button appears in the player; turn it on.
   Phone app: player settings > Captions.
3. Record: where the text appears (top or bottom), whether 28 characters
   per row is comfortable on a phone, accents and symbols, Clear, and
   whether it still works after stopping and restarting the stream.

Twitch's bandwidth-test mode hides the stream from viewers, so captions
cannot be checked there; use a quiet time or a test channel.

## Report back

- Which levels you ran, and the results (screenshots help)
- Encoder, recording format, Enhanced Broadcasting on/off
- Any OBS log lines (Help > Log Files > Show Current Log) mentioning
  `simple-local-captions`

These answer open questions 4, 5, 11 and 19.
