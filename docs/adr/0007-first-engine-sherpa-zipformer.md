# ADR 0007: First engine backend is sherpa-onnx with a streaming zipformer

Status: accepted (2026-10-13)

## Context

Roadmap phase 4 calls for a CPU streaming backend chosen from eval/
results. The eval harness does not exist yet, but the maintainer asked for
a usable model now. ADR 0002 (CPU-first) and ADR 0004 (no Whisper by
default) apply.

## Decision

- Runtime: **sherpa-onnx 1.13.8** C API with **ONNX Runtime** (CPU),
  prebuilt for Windows x64, used only inside the engine process.
  Apache-2.0 / MIT; approved by the maintainer (rule 3, 2026-10-13).
- Model: **sherpa-onnx-streaming-zipformer-en-2023-06-21** (trained on
  LibriSpeech audiobooks plus GigaSpeech podcasts and YouTube, closer to
  streamer speech than audiobook-only models; icefall
  libri-giga-pruned-transducer-stateless7-streaming-2023-04-04), int8
  encoder and joiner. Output is ALL CAPS without punctuation; the
  engine converts it to sentence case without changing words.
- Settings: 1 inference thread (as fast as 2, no spinning workers),
  greedy search, endpoints after 0.8 s of silence following speech or
  15 s of continuous speech. Partials report a stable prefix (words
  unchanged across 3 results), and the plugin shows stable partials, so
  captions appear while someone is still talking.

Measured on the maintainer's machine: real-time factor 0.12 on the
model's test files, about 11% of one CPU core while listening, about
150 MB RAM, about 9 s to load.

## Consequences

- No punctuation or real casing yet; a model with punctuation (for
  example NVIDIA Nemotron streaming, ~450 MB) is a candidate once eval/
  exists and its license is checked.
- English only.
- The model (~190 MB of int8 files) is not in the repo; until the installer
  exists it is downloaded by hand into
  `%LOCALAPPDATA%\simple-local-captions\models\`.
- The backend sits behind the protocol, so replacing it touches only
  `engine/asr-sherpa.*`.
