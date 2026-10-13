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

static pthread_mutex_t mutex;
static obs_output_t *outputs[CAPTION_TARGET_COUNT];
static caption_preview_cb preview_cb;
static void *preview_data;
/* UI thread only: guards against use after shutdown (e.g. a dock deleted
 * after module unload, which OBS 30.2 does not do, UI/obs-app.cpp). */
static bool ready;

bool caption_out_init(void)
{
	pthread_mutex_init_value(&mutex);
	if (pthread_mutex_init(&mutex, NULL) != 0) {
		obs_log(LOG_ERROR, "caption output: mutex init failed");
		return false;
	}
	ready = true;
	return true;
}

void caption_out_shutdown(void)
{
	for (int i = 0; i < CAPTION_TARGET_COUNT; i++)
		caption_out_detach((enum caption_target)i);
	caption_out_set_preview(NULL, NULL);
	ready = false;
	pthread_mutex_destroy(&mutex);
}

void caption_out_attach(enum caption_target target, obs_output_t *output)
{
	pthread_mutex_lock(&mutex);
	obs_output_release(outputs[target]);
	outputs[target] = obs_output_get_ref(output);
	pthread_mutex_unlock(&mutex);
}

void caption_out_detach(enum caption_target target)
{
	pthread_mutex_lock(&mutex);
	obs_output_release(outputs[target]);
	outputs[target] = NULL;
	pthread_mutex_unlock(&mutex);
}

void caption_out_set_preview(caption_preview_cb callback, void *data)
{
	if (!ready)
		return;
	pthread_mutex_lock(&mutex);
	preview_cb = callback;
	preview_data = data;
	pthread_mutex_unlock(&mutex);
}

unsigned caption_out_send(const char *text, double display_duration_s)
{
	unsigned sent = 0;

	pthread_mutex_lock(&mutex);
	for (int i = 0; i < CAPTION_TARGET_COUNT; i++) {
		if (outputs[i] && obs_output_active(outputs[i])) {
			obs_output_output_caption_text2(outputs[i], text, display_duration_s);
			sent |= 1u << i;
		}
	}
	if (preview_cb)
		preview_cb(text, sent, preview_data);
	pthread_mutex_unlock(&mutex);
	return sent;
}
