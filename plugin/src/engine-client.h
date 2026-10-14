/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Engine client (Windows): runs simple-local-captions-engine.exe from
 * <plugin>/bin/64bit/engine/ and talks to it over a named pipe
 * (protocol/PROTOCOL.md v0).
 *
 * On its own thread it drains the audio tap's ring buffer every 20 ms,
 * resamples to 16 kHz mono (libobs audio resampler) and sends audio frames;
 * reads hypotheses and feeds the core caption pipeline; ticks the pipeline
 * every 100 ms and hands updates to caption-out.c. If the engine exits it
 * is restarted with backoff (1 s doubling to 30 s); OBS keeps running.
 *
 * The model lives in %LOCALAPPDATA%\simple-local-captions\models\ (the
 * installer will put it there, installer/README.md).
 *
 * start/stop/request_reset are called on the UI thread.
 */

#include <stdbool.h>

bool engine_client_init(void);
void engine_client_shutdown(void);

/* Start listening (the audio tap gained an owner). No-op if running. */
void engine_client_start(void);
/* Stop the thread and the engine process. */
void engine_client_stop(void);

/* Forget caption state on the next tick (stream start). */
void engine_client_request_reset(void);

/* Locale key describing the engine state in plain language. */
const char *engine_client_status_key(void);
