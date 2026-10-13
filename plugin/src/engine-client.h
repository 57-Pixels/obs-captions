/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/*
 * Engine client: launches the simple-local-captions-engine process, speaks the named
 * pipe protocol in protocol/PROTOCOL.md, forwards audio frames from the ring
 * buffer and hands hypothesis events to the core caption pipeline.
 *
 * Runs on its own thread; never on the OBS audio or UI thread.
 *
 * Stub: no behaviour until roadmap phase 3.
 */

#include <stdbool.h>

bool engine_client_init(void);
void engine_client_shutdown(void);
