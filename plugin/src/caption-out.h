/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Caption output: the ONLY module allowed to call
 * obs_output_output_caption_text2 (libobs/obs.h).
 *
 * Facts from libobs 30.2.3 (libobs/obs-output.c) this module respects:
 * - Captions are queued per output in a FIFO. display_duration is the
 *   minimum time before the NEXT queued caption is sent, so callers send at
 *   most one update per interval (core scheduler, ADR 0003).
 * - Text is truncated to CAPTION_LINE_BYTES (128 bytes, obs-internal.h).
 * - The queue is freed when the output stops; frontend.c re-attaches the
 *   streaming output on every stream start.
 *
 * Threading: attach/detach are called on the UI thread (frontend events);
 * caption_out_send may be called from any thread. A mutex guards the
 * output reference.
 */

#include <stdbool.h>

#include <obs.h>

#ifdef __cplusplus
extern "C" {
#endif

enum caption_out_result {
	CAPTION_OUT_SENT,
	CAPTION_OUT_NOT_STREAMING,
};

bool caption_out_init(void);
void caption_out_shutdown(void);

/* Takes its own reference to output (the stream output that just started). */
void caption_out_attach(obs_output_t *output);
/* Releases the stream output; sends fail with NOT_STREAMING until attached. */
void caption_out_detach(void);

/*
 * text: UTF-8 using only CEA-608 characters, rows separated by '\n', at
 * most 128 bytes; "" clears the screen. display_duration_s: see above.
 */
enum caption_out_result caption_out_send(const char *text, double display_duration_s);

#ifdef __cplusplus
}
#endif
