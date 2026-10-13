# eval/: regression harness (plan, no code yet)

Captions are judged by streamers and viewers, not by unit tests alone. This
harness decides which ASR backend ships (ADR 0002) and catches regressions.

## Corpus

- 30-60 s clips of real streaming audio with exact reference transcripts:
  game audio + voice, music beds, loud keyboards, laughing/shouting, long
  silence, accents. Only clips whose license allows redistribution, or
  private clips kept out of the repo.
- A "silence and music" set whose reference is empty.
- A slang/casual-speech set with references written as spoken, not
  corrected, so the harness can catch models that normalize speech.

## Metrics (per backend, per clip, plus p50/p95)

| Metric | Target for the default tier |
|--------|-----------------------------|
| WER on gamer audio | report; lower is better |
| Verbatim rate: casual/slang phrases kept as said ("wuz up", "gonna") | report; models that rewrite speech into formal text lose points |
| Captions emitted on silence/music clips | **0** |
| Caption lag (speech end to caption shown), p95 | < 2.5 s |
| Engine CPU % on a 6-core laptop | < 15 % average |
| OBS dropped/lagged frames vs. captions off | no increase |

## How it will run

1. Replay clips through the engine (no OBS) to an event log (protocol JSON).
2. Feed event logs through core/ with an injected clock to get a caption
   timeline.
3. Score timelines against references; write a Markdown/CSV report.
4. A separate manual run inside OBS measures dropped/lagged frames.

Reports are committed under `eval/reports/` so backend choices are
traceable.
