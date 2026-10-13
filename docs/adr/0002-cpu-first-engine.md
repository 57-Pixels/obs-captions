# ADR 0002: CPU-first default engine, GPU as optional tiers

Status: accepted (2026-10-12)

## Context

Target streamers run slightly below the Steam Hardware Survey average: 8 GB
GPU, 6-8 core CPU, 16 GB RAM, often laptops. The GPU is already busy with the
game and, usually, the video encoder (NVENC/AMF/QSV). GPU inference competes
for the same VRAM and scheduling and risks dropped or lagged frames, which is
the one failure a streamer will not forgive.

## Decision

The default tier is a streaming ASR model that runs in real time on 1-2 CPU
cores at below-normal priority. GPU backends are optional tiers the user
opts into in the Advanced panel, after a health check passes. The concrete
backend is chosen from eval/ results (roadmap phase 4/5), not in this ADR.

## Consequences

- Accuracy of the default tier is capped by what a small CPU model can do;
  filters and the word list (core/) carry more weight.
- The eval harness must measure CPU % and OBS dropped/lagged frames, not only
  WER.
- Users with spare GPU capacity can choose a better tier explicitly.
