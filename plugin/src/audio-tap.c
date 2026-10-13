/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "audio-tap.h"

#include <math.h>
#include <stdio.h>

/* struct obs_source_info, OBS_SOURCE_TYPE_FILTER, OBS_SOURCE_AUDIO,
 * obs_register_source: libobs/obs-source.h; struct obs_audio_data,
 * obs_filter_get_parent, obs_source_get_name, obs_get_audio: libobs/obs.h;
 * obs_module_text: libobs/obs-module.h */
#include <obs-module.h>
/* obs_properties_create, obs_properties_add_button, obs_properties_add_text,
 * obs_property_text_set_info_type, OBS_TEXT_INFO: libobs/obs-properties.h */
#include <obs-properties.h>
/* audio_output_get_channels: libobs/media-io/audio-io.h */
#include <media-io/audio-io.h>
/* os_gettime_ns: libobs/util/platform.h */
#include <util/platform.h>
/* os_atomic_set_long, os_atomic_load_long: libobs/util/threading.h */
#include <util/threading.h>
/* bzalloc, bfree: libobs/util/bmem.h */
#include <util/bmem.h>

#include <plugin-support.h>

#include "caption-test.h"

#define FILTER_ID "simple_local_captions_filter"

/* Audio counts as "arriving" if a block came within this many seconds. */
#define AUDIO_RECENT_S 1

struct audio_tap {
	obs_source_t *filter;
	size_t channels;

	/* Written by the audio thread, read by the UI thread. */
	volatile long peak_milli;   /* peak sample of the last block x 1000 */
	volatile long last_audio_s; /* os_gettime_ns() / 1e9 of the last block */

	/* UI thread only. */
	const char *last_result_key;
};

static long now_s(void)
{
	return (long)(os_gettime_ns() / 1000000000ULL);
}

static const char *tap_get_name(void *type_data)
{
	(void)type_data;
	return obs_module_text("Filter.Name");
}

static void *tap_create(obs_data_t *settings, obs_source_t *filter)
{
	(void)settings;
	struct audio_tap *tap = bzalloc(sizeof(*tap));
	tap->filter = filter;
	tap->channels = audio_output_get_channels(obs_get_audio());
	tap->last_audio_s = -1;
	return tap;
}

static void tap_destroy(void *data)
{
	bfree(data);
}

/* Audio thread. Filters receive float planar samples, one plane per
 * channel. Pass audio through unchanged; only measure it. */
static struct obs_audio_data *tap_filter_audio(void *data, struct obs_audio_data *audio)
{
	struct audio_tap *tap = data;
	float peak = 0.0f;

	for (size_t ch = 0; ch < tap->channels && ch < MAX_AV_PLANES; ch++) {
		const float *samples = (const float *)audio->data[ch];
		if (!samples)
			continue;
		for (uint32_t i = 0; i < audio->frames; i++) {
			float v = fabsf(samples[i]);
			if (v > peak)
				peak = v;
		}
	}

	os_atomic_set_long(&tap->peak_milli, (long)(peak * 1000.0f));
	os_atomic_set_long(&tap->last_audio_s, now_s());
	return audio;
}

static void format_audio_status(struct audio_tap *tap, char *out, size_t size)
{
	obs_source_t *parent = obs_filter_get_parent(tap->filter);
	const char *name = parent ? obs_source_get_name(parent) : "?";
	long last = os_atomic_load_long(&tap->last_audio_s);

	if (last < 0 || now_s() - last > AUDIO_RECENT_S) {
		snprintf(out, size, obs_module_text("Filter.Status.NoAudio"), name);
		return;
	}

	long peak_milli = os_atomic_load_long(&tap->peak_milli);
	if (peak_milli <= 0) {
		snprintf(out, size, obs_module_text("Filter.Status.Silent"), name);
		return;
	}
	int db = (int)lround(20.0 * log10((double)peak_milli / 1000.0));
	snprintf(out, size, obs_module_text("Filter.Status.Hearing"), name, db);
}

static bool on_send_test(obs_properties_t *props, obs_property_t *property, void *data)
{
	(void)props;
	(void)property;
	struct audio_tap *tap = data;
	tap->last_result_key = caption_test_result_key(caption_test_send_next());
	return true; /* rebuild properties so the status updates */
}

static bool on_clear(obs_properties_t *props, obs_property_t *property, void *data)
{
	(void)props;
	(void)property;
	struct audio_tap *tap = data;
	tap->last_result_key = caption_test_result_key(caption_test_clear());
	return true;
}

static bool on_refresh(obs_properties_t *props, obs_property_t *property, void *data)
{
	(void)props;
	(void)property;
	(void)data;
	return true;
}

static obs_properties_t *tap_get_properties(void *data)
{
	struct audio_tap *tap = data;
	obs_properties_t *props = obs_properties_create();
	char status[256];

	format_audio_status(tap, status, sizeof(status));
	obs_property_t *p = obs_properties_add_text(props, "audio_status", status, OBS_TEXT_INFO);
	obs_property_text_set_info_type(p, OBS_TEXT_INFO_NORMAL);
	obs_properties_add_button(props, "refresh", obs_module_text("Filter.Refresh"), on_refresh);

	obs_properties_add_button(props, "send_test", obs_module_text("Test.Send"), on_send_test);
	obs_properties_add_button(props, "clear", obs_module_text("Test.Clear"), on_clear);
	if (tap->last_result_key)
		obs_properties_add_text(props, "test_result", obs_module_text(tap->last_result_key), OBS_TEXT_INFO);

	obs_properties_add_text(props, "help", obs_module_text("Filter.Help"), OBS_TEXT_INFO);
	return props;
}

static struct obs_source_info audio_tap_info = {
	.id = FILTER_ID,
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = tap_get_name,
	.create = tap_create,
	.destroy = tap_destroy,
	.filter_audio = tap_filter_audio,
	.get_properties = tap_get_properties,
};

bool audio_tap_init(void)
{
	obs_register_source(&audio_tap_info);
	return true;
}

void audio_tap_shutdown(void) {}
