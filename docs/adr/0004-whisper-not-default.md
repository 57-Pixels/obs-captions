# ADR 0004: Whisper is not the default engine

Status: accepted (2026-10-12)

## Context

Whisper-family models are the most common choice in existing OBS caption
plugins. They are trained on 30 s windows, not streaming audio, so live use
needs repeated re-decoding of overlapping windows (high CPU/GPU cost, unstable
partials). They hallucinate on silence and music ("Thank you.", "Thanks for
watching!", subtitle credits). The models that are accurate enough are heavy
for the target hardware (ADR 0002).

## Decision

The default engine is a native streaming ASR model. Whisper may be added
later as an optional backend behind the same protocol, for users who accept
its cost.

## Consequences

- We must evaluate streaming models ourselves (eval/), since there is less
  community tooling.
- The hallucination filter in core/ is still needed (any model can produce
  silence artifacts) and is required if a Whisper backend is ever added.
