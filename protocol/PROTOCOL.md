# Plugin <-> engine protocol, v0

Status: v0 subset implemented (2026-10-13). Mirrors `engine/engine-api.h`.

Implemented now: transport, hello / hello_ack, audio frames, hypothesis,
status (`listening`), error (`model_missing`, `protocol`), and shutdown.
Not yet: ping/pong heartbeat (the plugin instead watches the engine
process handle and the pipe for EOF), `rtf`/`cpu_pct` in status, and the
load_model / set_tier / set_word_list controls (the engine gets its model
directory on the command line).

## Transport

- Windows named pipe `\\.\pipe\simple-local-captions-<plugin-pid>-<time>`,
  created by the plugin (server) before it launches the engine; the name is
  passed on the engine command line (`--pipe <name> --model <dir>`). One
  instance only (`FILE_FLAG_FIRST_PIPE_INSTANCE`) and local clients only
  (`PIPE_REJECT_REMOTE_CLIENTS`); default pipe ACL.
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
