/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * C interface to CaptionPipeline for the plugin (written in C). Thin
 * wrapper only: behaviour is defined and tested in the C++ API
 * (slc/pipeline.h). Same threading rule: one thread per pipeline.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum caption text length in bytes (libobs CAPTION_LINE_BYTES). */
#define SLC_CAPTION_MAX_BYTES 128

typedef struct slc_pipeline slc_pipeline;

/* Default configuration; show_stable_partial: 1 to show the stable part of
 * partial hypotheses (for engines that report a stable prefix). */
slc_pipeline *slc_pipeline_create(int show_stable_partial);
void slc_pipeline_destroy(slc_pipeline *pipeline);

/* is_final: 1 for a final, 0 for a partial. text: UTF-8. */
void slc_pipeline_push(slc_pipeline *pipeline, int is_final, const char *text, int64_t t_start_ms, int64_t t_end_ms,
		       size_t stable_prefix_len);

/*
 * Returns 1 and fills text (NUL-terminated; "" = clear the screen) and
 * *display_duration_ms when a caption update is due, else 0. text_size
 * should be at least SLC_CAPTION_MAX_BYTES + 1.
 */
int slc_pipeline_tick(slc_pipeline *pipeline, int64_t now_ms, char *text, size_t text_size,
		      int64_t *display_duration_ms);

/* Forget caption state (stream start). */
void slc_pipeline_reset(slc_pipeline *pipeline);

#ifdef __cplusplus
}
#endif
