/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "frontend.h"

/* obs_frontend_add_event_callback, obs_frontend_remove_event_callback,
 * obs_frontend_get_streaming_output, enum obs_frontend_event:
 * UI/obs-frontend-api/obs-frontend-api.h */
#include <obs-frontend-api.h>
/* obs_output_release: libobs/obs.h */
#include <obs.h>

#include <plugin-support.h>

#include "caption-out.h"

static void on_stream_started(void)
{
	/* Returns a new reference (UI/api-interface.cpp, obs_output_get_ref);
	 * for Enhanced Broadcasting it is the multitrack output. */
	obs_output_t *output = obs_frontend_get_streaming_output();
	if (!output) {
		obs_log(LOG_WARNING, "stream started but no streaming output found");
		return;
	}
	caption_out_attach(output);
	obs_output_release(output);
	obs_log(LOG_INFO, "stream started: captions attached");
}

static void on_frontend_event(enum obs_frontend_event event, void *data)
{
	(void)data;

	switch (event) {
	case OBS_FRONTEND_EVENT_STREAMING_STARTED:
		on_stream_started();
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
	case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
	case OBS_FRONTEND_EVENT_EXIT:
		caption_out_detach();
		break;
	default:
		break;
	}
}

bool frontend_init(void)
{
	obs_frontend_add_event_callback(on_frontend_event, NULL);
	return true;
}

void frontend_shutdown(void)
{
	obs_frontend_remove_event_callback(on_frontend_event, NULL);
}
