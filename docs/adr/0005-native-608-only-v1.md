# ADR 0005: v1 outputs native CEA-608 only

Status: accepted (2026-10-12)

## Context

Twitch shows embedded CEA-608 captions natively (viewer CC button, including
mobile apps). OBS writes them via libcaption as CC1 pop-on captions. 608 is
limited: Latin script, 32 columns, a fixed character set, plain text.

## Decision

v1 outputs are:

1. Native CEA-608 CC1, pop-on, plain text, Latin script.
2. Optional on-screen text source (for platforms without CC support).
3. Optional transcript file.

Not in v1: translation, cloud APIs, WebVTT/sidecar formats, 708, non-Latin
scripts, styling or positioning.

## Consequences

- The formatter transliterates to the 608 character set and strips the rest
  (counted for diagnostics).
- Non-Latin languages are out of scope for v1; the UI must say so instead of
  producing broken output.
- Default layout is 2 rows x 28 columns for mobile legibility (hard max 32).
