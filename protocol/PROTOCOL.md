# Plugin <-> engine protocol, v0

Status: draft, not implemented. Mirrors `engine/engine-api.h`.

## Transport

- Windows named pipe `\.\pipe\simple-local-captions-<plugin-pid>-<random>`, created
  by the plugin (server) before it launches the engine; the name is passed
  on the engine command line. One client only; the pipe ACL is limited to
  the current user.
- Duplex, byte mode. Two logical streams share it, distinguished by a
  1-byte frame type:
  - `0x01` **binary frame** (audio), plugin -> engine
  - `0x02` **JSON line** (UTF-8, `\n`-terminated), either direction
- All integers little-endian.

## Handshake

1. Engine -> plugin, within 5 s of connect:
   ```json
   {"type":"hello","protocol":0,"engine_version":"0.1.0",
    "capabilities":{"backends":["cpu-default"],"partials":true,
                    "stable_prefix":true,"word_list":true,"languages":["en"]}}
   ```
2. Plugin -> engine:
   ```json
   {"type":"hello_ack","protocol":0,"plugin_version":"1.0.0"}
   ```
   If major protocol numbers differ, the plugin sends `shutdown` and shows
   the `Engine.Error.Version` message.

## Heartbeat

Each side sends `{"type":"ping","seq":N}` every 1 s; the peer answers
`{"type":"pong","seq":N}`. Three missed pongs (3 s) = peer dead. The plugin
then kills and restarts the engine with exponential backoff (1, 2, 4 ... 30 s).

## Audio frames (plugin -> engine)

| Offset | Size | Field | Notes |
|-------:|-----:|-------|-------|
| 0 | 1 | frame type | `0x01` |
| 1 | 4 | magic | `"OCAF"` |
| 5 | 4 | seq | u32, +1 per frame, wraps |
| 9 | 8 | audio_clock_ns | u64, OBS audio timestamp of first sample |
| 17 | 4 | n_samples | u32, typically 320 (20 ms) |
| 21 | 2*n | samples | s16le, 16 kHz, mono |

A gap in `seq` means frames were dropped (ring buffer overrun); the engine
treats it as silence and reports it in `status`.

## Engine -> plugin events (JSON lines)

```json
{"type":"hypothesis","kind":"partial","text":"so what we're going to",
 "t_start":12.48,"t_end":13.92,"stable_prefix_len":14}
{"type":"hypothesis","kind":"final","text":"So what we're going to do is",
 "t_start":12.48,"t_end":14.40,"stable_prefix_len":28}
{"type":"status","model":"cpu-default-en","rtf":0.21,"cpu_pct":11.5,
 "state":"listening"}
{"type":"error","code":"model_missing","message":"model file not found: ...",
 "user_message_key":"Engine.Error.ModelMissing"}
```

- `t_start`/`t_end`: seconds on the audio clock (derived from
  `audio_clock_ns`), so the plugin can measure lag.
- `stable_prefix_len`: count of UTF-8 bytes at the start of `text` the engine
  promises not to change. Finals have `stable_prefix_len == len(text)`.
- `state`: `loading | listening | paused | degraded`.
- `user_message_key`: key in `plugin/data/locale/*.ini`; `message` is for logs.

## Control messages (plugin -> engine, JSON lines)

```json
{"type":"load_model","model_id":"cpu-default-en","path":"..."}
{"type":"set_tier","tier":"cpu"}
{"type":"set_word_list","words":["Valorant","GG","Pog"]}
{"type":"shutdown"}
```

Each control message gets `{"type":"ack","for":"<type>","ok":true}` or an
`error` event.

## Text input for TTS / VTuber apps (documented only, not implemented)

A localhost WebSocket lets an app that *knows* the exact spoken line push it
directly. These lines bypass ASR and take priority in the scheduler while
their estimated duration is active.

- Endpoint: `ws://127.0.0.1:<port>/simple-local-captions/v0` (port configurable,
  default off). Bound to loopback only; requires a token shown in the dock.
- Message:
  ```json
  {"text":"Thanks for the raid!","start_time":1234.56,
   "est_duration":2.4,"speaker":"Mika"}
  ```
  `start_time`: seconds, sender clock (the plugin uses receive time if it is
  more than 1 s off); `est_duration`: seconds; `speaker`: optional.
- Handled by the plugin, not the engine; delivered to core as a text event.
