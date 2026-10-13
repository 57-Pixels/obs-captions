/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "caption-out.h"

/* obs_output_get_ref, obs_output_release, obs_output_active,
 * obs_output_output_caption_text2: libobs/obs.h */
#include <obs.h>
/* pthread_mutex_*: libobs/util/threading.h (w32-pthreads on Windows) */
#include <util/threading.h>

#include <plugin-support.h>

static pthread_mutex_t output_mutex;
static obs_output_t *stream_output;

bool caption_out_init(void)
{
	pthread_mutex_init_value(&output_mutex);
	if (pthread_mutex_init(&output_mutex, NULL) != 0) {
		obs_log(LOG_ERROR, "caption output: mutex init failed");
		return false;
	}
	return true;
}

void caption_out_shutdown(void)
{
	caption_out_detach();
	pthread_mutex_destroy(&output_mutex);
}

void caption_out_attach(obs_output_t *output)
{
	pthread_mutex_lock(&output_mutex);
	obs_output_release(stream_output);
	stream_output = obs_output_get_ref(output);
	pthread_mutex_unlock(&output_mutex);
}

void caption_out_detach(void)
{
	pthread_mutex_lock(&output_mutex);
	obs_output_release(stream_output);
	stream_output = NULL;
	pthread_mutex_unlock(&output_mutex);
}

enum caption_out_result caption_out_send(const char *text, double display_duration_s)
{
	enum caption_out_result result = CAPTION_OUT_NOT_STREAMING;

	pthread_mutex_lock(&output_mutex);
	if (stream_output && obs_output_active(stream_output)) {
		obs_output_output_caption_text2(stream_output, text, display_duration_s);
		result = CAPTION_OUT_SENT;
	}
	pthread_mutex_unlock(&output_mutex);
	return result;
}
