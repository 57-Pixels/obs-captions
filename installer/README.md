# installer/: plan (not implemented)

Goal: install in one click and never leave a half-working setup.

## Installer

- Signed Windows installer; the DLL and engine EXE are signed too.
- Detects the OBS install and version; refuses OBS older than 30.2 with a
  plain-language message.
- Installs to `C:\ProgramData\obs-studio\plugins\obs-captions\`
  (`bin\64bit\` for the DLL and engine, `data\` for locale).
- Clean uninstall; asks before deleting downloaded models.

## Model downloader

- Models are **not** bundled. The first-run sheet downloads the default CPU
  model after explicit consent, showing size and source.
- Downloads come from a pinned URL list with a SHA-256 per file, are
  resumable, and are verified before use. Models live in
  `%LOCALAPPDATA%\obs-captions\models\`.
- A model's license is recorded in `docs/DEPENDENCIES.md` before it is
  offered.

## Updates

- Opt-in update check (no telemetry); signed update packages only.
