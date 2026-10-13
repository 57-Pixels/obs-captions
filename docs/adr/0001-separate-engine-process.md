# ADR 0001: Run speech recognition in a separate engine process

Status: accepted (2026-10-12)

## Context

Speech recognition needs large native runtimes (ONNX Runtime, possibly
CUDA/DirectML, model files of hundreds of MB). Running them inside obs64.exe
means any crash, out-of-memory or driver fault takes the stream down; their
DLLs can clash with other plugins that bundle different versions (a known
problem with in-process ASR plugins); and the OS scheduler cannot tell
"captions" apart from "encoding".

## Decision

ASR runs in `simple-local-captions-engine.exe`, launched and supervised by the plugin
(`engine-client.c`). They talk over a Windows named pipe
([protocol/PROTOCOL.md](../../protocol/PROTOCOL.md)). The engine runs at
`BELOW_NORMAL_PRIORITY_CLASS`. The plugin never loads an inference runtime.

## Consequences

- An engine crash pauses captions; OBS keeps streaming. The plugin restarts
  the engine with backoff and reports it in plain language.
- Each backend can ship its own DLLs next to the engine without touching OBS.
- Cost: an IPC protocol to version and test, ~1-2 ms extra latency per audio
  frame (negligible against ASR latency), and a second binary to sign.
- The protocol lets a fake engine replay recorded transcripts, which makes
  plugin tests possible without a model (roadmap phase 3).
