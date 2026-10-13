/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/*
 * Caption output: the ONLY module allowed to call
 * obs_output_output_caption_text2 (libobs/obs.h).
 *
 * Facts from libobs 30.2.3 (libobs/obs-output.c) this module must respect:
 * - Captions are queued per output in a FIFO. display_duration is the
 *   minimum time before the NEXT queued caption is sent, so the core
 *   scheduler sends at most one update per interval (ADR 0003).
 * - Text is truncated to CAPTION_LINE_BYTES (128 bytes, obs-internal.h).
 * - The queue is cleared when the output stops, so caption state is re-armed
 *   on every stream start (see frontend.c).
 *
 * Stub: no behaviour until roadmap phase 2.
 */

#include <stdbool.h>

bool caption_out_init(void);
void caption_out_shutdown(void);
