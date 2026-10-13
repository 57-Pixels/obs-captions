# obs-captions

Live, local speech-to-text for OBS Studio on Windows, sent to Twitch as
**native closed captions** (CEA-608, CC1) that viewers switch on with the CC
button, including on phones watching muted.

> Status: **early development.** The plugin loads and logs, and the caption
> logic (`core/`) is tested, but nothing is sent to the stream yet. See the
> roadmap.

## Why

Very few channels send captions. The tools that exist are hard to set up,
cost frames, or show embarrassing output ("Thank you." on silence). The goal
is an experience that just works: one switch, correct defaults,
plain-language status, and an Advanced panel for power users.

- **Local only.** Speech recognition runs on the streamer's PC. No cloud.
- **CPU-first.** Built for average streaming PCs and laptops; captions must
  never cost the stream frames ([ADR 0002](docs/adr/0002-cpu-first-engine.md)).
- **Isolated.** Recognition runs in its own process; if it crashes, the
  stream keeps going ([ADR 0001](docs/adr/0001-separate-engine-process.md)).

## Requirements

- Windows 10/11 x64
- OBS Studio 30.2 or newer ([ADR 0006](docs/adr/0006-minimum-obs-version.md))

## Layout

| Path | What |
|------|------|
| `plugin/` | Thin OBS plugin (C, Qt for the dock) |
| `core/` | Caption logic: filters, formatter, scheduler. Pure C++17, no OBS |
| `engine/` | Separate speech-recognition process |
| `protocol/` | Plugin <-> engine protocol |
| `eval/`, `installer/` | Plans for the evaluation harness and installer |
| `docs/` | [Architecture](docs/ARCHITECTURE.md), [ADRs](docs/adr/), [dependencies](docs/DEPENDENCIES.md), [open questions](docs/OPEN_QUESTIONS.md) |

## Roadmap

0. Scaffold from obs-plugintemplate; plugin loads (done)
1. `core/` caption library with golden-file tests (done)
2. `caption-out` + "Send test caption" dock button (verify on Twitch desktop
   and phone)
3. `audio-tap` + ring buffer + `engine-client` against a fake engine that
   replays recorded transcripts
4. Engine process with the first real CPU streaming backend (chosen from
   eval/ results)
5. eval/ harness: gamer-audio WER, zero captions on silence, p95 lag, CPU %,
   OBS dropped frames
6. Simple dock UX, first-run sheet, health checks, post-stream summary
7. Advanced panel, word list UI, model manager
8. Signed installer, auto-update, model downloader

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) and [AGENTS.md](AGENTS.md). Much of
this code is AI-written, so this project is a standalone plugin and is not
intended for inclusion in OBS Studio itself. It is structured so the
OBS-facing parts are small enough to review or rewrite by hand.

## License

GPL-2.0-or-later. See [LICENSE](LICENSE). Every source file carries an SPDX
header.
