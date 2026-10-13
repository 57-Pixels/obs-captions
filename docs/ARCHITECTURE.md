# Architecture

simple-local-captions turns a streamer's microphone audio into native CEA-608 closed
captions (CC1) on the OBS stream output. It is built from three parts, kept
apart so that each can be understood, tested and replaced on its own.

## The three parts

### 1. Thin plugin (`plugin/`, C, minimal C++ for Qt)

The only code that talks to OBS. It does as little as possible:

| Module            | Responsibility                                              |
|-------------------|-------------------------------------------------------------|
| `plugin-main.c`   | Module load/unload and registration. Nothing else.          |
| `audio-tap.c`     | Copy audio from the chosen source into an SPSC ring buffer. |
| `engine-client.c` | Run the engine process, stream audio, receive events.       |
| `caption-out.c`   | The **only** caller of `obs_output_output_caption_text2`.   |
| `frontend.c`      | Stream start/stop events, health checks, re-arming state.   |
| `ui/`             | Qt dock: one switch, plain-language status.                 |

Why thin: OBS-facing code is the hardest to test and the part a human may
need to rewrite by hand (see README). Keeping it small keeps that cheap.

### 2. Core library (`core/`, C++17, zero OBS dependency)

Pure, deterministic logic: filters, formatter, caption scheduler. Time is
always passed in (`tick(now)`), never read from a clock, so every behaviour is
reproducible in golden-file tests. The plugin links it statically.

Why separate: caption quality ("no embarrassing output") lives here, and it
must be testable on any machine without OBS, audio, or a model.

### 3. Engine process (`engine/`, C++)

A separate executable that runs speech recognition and talks to the plugin
over a named pipe ([protocol/PROTOCOL.md](../protocol/PROTOCOL.md)).

Why a process ([ADR 0001](adr/0001-separate-engine-process.md)):

- **Crash isolation.** If the ASR runtime crashes or runs out of memory, OBS
  keeps streaming; the plugin shows "Captions paused" and restarts the engine.
- **No DLL conflicts.** Inference runtimes (ONNX Runtime, CUDA, etc.) ship
  DLLs that other OBS plugins also load in different versions.
- **Priority control.** The engine runs at below-normal priority so encoding
  and rendering always win; captions degrade before the stream does.
- **Swappable backends.** CPU-first default, optional GPU tiers
  ([ADR 0002](adr/0002-cpu-first-engine.md)), all behind one protocol.

## Data flow

```
 OBS audio thread                 engine-client thread              engine.exe
 ----------------                 --------------------              ----------
 audio source
   | (callback: copy + return)
   v
 audio-tap.c --> [SPSC ring buffer] --> resample 16 kHz mono --pipe--> ASR backend
                                                                          |
                     hypothesis / status / error (JSON lines) <--pipe-----+
                                    |
                                    v
                         core: filters -> scheduler -> formatter
                                    |
                     (tick every ~100 ms; latest-wins slot)
                                    |
                                    v  optional CaptionUpdate
                              caption-out.c
                                    |
                                    v
              obs_output_output_caption_text2(stream output, text, duration)
                                    |
                                    v
                    libobs caption FIFO -> CEA-608 in video SEI/OBU -> Twitch

 Optional, documented only: TTS/VTuber app --WebSocket--> text events --> core
```

## Threading rules

Full list in [AGENTS.md](../AGENTS.md). In short:

- **OBS audio callback:** copy into the lock-free SPSC ring buffer and return.
  No blocking, no heavy allocation, no contended locks, no I/O, no logging in
  the hot path.
- **engine-client thread:** owns the pipe, the core pipeline and the tick.
  It is the only thread that calls into `core/`.
- **Caption output:** `caption-out.c` may be called only from the
  engine-client thread; it takes a reference to the stream output, sends,
  and releases it.
- **Qt UI thread:** the only thread that touches Qt objects. Other threads
  post status to it (queued signal or `QMetaObject::invokeMethod`).
- **References:** every `obs_*_get_*` that returns a reference is released in
  the same function.

## Caption constraints from libobs 30.2.3

Verified in `libobs/obs-output.c` and `libobs/obs-internal.h`:

- Text captions are queued per output in a FIFO. `display_duration` is the
  minimum time before the *next* queued caption is sent. Queuing every
  partial therefore causes growing lag ([ADR 0003](adr/0003-latest-wins-scheduler.md)).
- Text is truncated to `CAPTION_LINE_BYTES` (128 bytes) per call.
- libcaption (`deps/libcaption/src/caption.c`) renders up to 15 rows of up to
  32 columns, splitting on `\n` and wrapping at spaces.
- The caption queue is freed when the output stops, so state is re-armed on
  every stream start.
