/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "frontend.h"

/* obs_frontend_add_event_callback, obs_frontend_remove_event_callback,
 * obs_frontend_get_streaming_output, obs_frontend_get_recording_output,
 * enum obs_frontend_event: UI/obs-frontend-api/obs-frontend-api.h */
#include <obs-frontend-api.h>
/* obs_output_release: libobs/obs.h */
#include <obs.h>

#include <plugin-support.h>

#include "caption-out.h"

/* Both getters return a new reference (UI/api-interface.cpp,
 * obs_output_get_ref); for Enhanced Broadcasting the streaming output is
 * the multitrack output. */
static void attach(enum caption_target target, obs_output_t *output, const char *what)
{
	if (!output) {
		obs_log(LOG_WARNING, "%s started but its output was not found", what);
		return;
	}
	caption_out_attach(target, output);
	obs_output_release(output);
	obs_log(LOG_INFO, "%s started: captions attached", what);
}

static void on_frontend_event(enum obs_frontend_event event, void *data)
{
	(void)data;

	switch (event) {
	case OBS_FRONTEND_EVENT_STREAMING_STARTED:
		attach(CAPTION_TARGET_STREAM, obs_frontend_get_streaming_output(), "stream");
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
	case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
		caption_out_detach(CAPTION_TARGET_STREAM);
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STARTED:
		attach(CAPTION_TARGET_RECORDING, obs_frontend_get_recording_output(), "recording");
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STOPPING:
	case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
		caption_out_detach(CAPTION_TARGET_RECORDING);
		break;
	case OBS_FRONTEND_EVENT_EXIT:
		caption_out_detach(CAPTION_TARGET_STREAM);
		caption_out_detach(CAPTION_TARGET_RECORDING);
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
