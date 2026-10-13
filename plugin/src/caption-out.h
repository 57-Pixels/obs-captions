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
 * Captions go to every attached output that is active: the stream (viewers
 * toggle CC) and the local recording (captions embedded in the file, so a
 * player such as VLC can show them; also a way to test without going live).
 * libobs embeds text captions for any output with a video encoder
 * (libobs/obs-output.c, set_video_encoder / send_interleaved).
 *
 * Facts from libobs 30.2.3 this module respects:
 * - Captions are queued per output in a FIFO. display_duration is the
 *   minimum time before the NEXT queued caption is sent, so callers send at
 *   most one update per interval (core scheduler, ADR 0003).
 * - Text is truncated to CAPTION_LINE_BYTES (128 bytes, obs-internal.h).
 * - The queue is freed when an output stops; frontend.c re-attaches outputs
 *   on every start.
 *
 * Threading: attach/detach run on the UI thread (frontend events);
 * caption_out_send may be called from any thread. A mutex guards the output
 * references and the preview listener.
 */

#include <stdbool.h>

#include <obs.h>

#ifdef __cplusplus
extern "C" {
#endif

enum caption_target {
	CAPTION_TARGET_STREAM,
	CAPTION_TARGET_RECORDING,
	CAPTION_TARGET_COUNT,
};

/* Bit flags returned by caption_out_send. 0 = no active output (preview
 * only). */
#define CAPTION_SENT_STREAM (1u << CAPTION_TARGET_STREAM)
#define CAPTION_SENT_RECORDING (1u << CAPTION_TARGET_RECORDING)

/*
 * Called after every caption_out_send with the text (possibly "" = clear),
 * on the sending thread, while caption-out's mutex is held: it must return
 * quickly and must not call back into caption-out. The dock uses it for its
 * preview and posts to the UI thread.
 */
typedef void (*caption_preview_cb)(const char *text, unsigned sent_flags, void *data);

bool caption_out_init(void);
void caption_out_shutdown(void);

/* Takes its own reference to output. */
void caption_out_attach(enum caption_target target, obs_output_t *output);
void caption_out_detach(enum caption_target target);

/* Only one listener; pass NULL to remove it. */
void caption_out_set_preview(caption_preview_cb callback, void *data);

/*
 * text: UTF-8 using only CEA-608 characters, rows separated by '\n', at
 * most 128 bytes; "" clears the screen. Returns CAPTION_SENT_* flags.
 */
unsigned caption_out_send(const char *text, double display_duration_s);

#ifdef __cplusplus
}
#endif
