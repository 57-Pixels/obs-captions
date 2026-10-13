# ADR 0003: Latest-wins caption scheduler instead of a queue

Status: accepted (2026-10-12)

## Context

`obs_output_output_caption_text2(output, text, display_duration)` appends to
a per-output FIFO (libobs 30.2.3, `libobs/obs-output.c`). A queued caption is
sent only once the previous one's `display_duration` has elapsed. Streaming
ASR emits several partial hypotheses per second. If every partial is queued,
the queue grows faster than it drains: captions fall further behind speech
and arrive in visible "batches".

## Decision

The core scheduler keeps a single slot holding the latest displayable text.
`tick(now)` emits at most one `CaptionUpdate` per minimum interval (default
1000 ms), only when the text changed. Older pending text is overwritten,
never queued. `display_duration` passed to OBS never exceeds the interval,
so the libobs FIFO holds at most one item.

## Consequences

- Lag is bounded by the interval plus ASR latency, independent of speech
  rate.
- Some intermediate wordings are never shown; viewers see fewer, more stable
  updates. This is intended.
- Only text that cannot be retracted is shown (committed finals plus,
  optionally, the stable prefix of the current partial).
