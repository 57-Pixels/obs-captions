/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "caption-test.h"

#include <stdbool.h>
#include <stddef.h>

/* obs_module_text: libobs/obs-module.h */
#include <obs-module.h>

#include <plugin-support.h>

#include "caption-out.h"

static const char *const test_caption_keys[] = {
	"Test.Caption.Plain",
	"Test.Caption.Accents",
};
#define TEST_CAPTION_COUNT (sizeof(test_caption_keys) / sizeof(test_caption_keys[0]))

/* Long enough to read; the next update may follow after this time. */
#define TEST_DISPLAY_S 3.0

static size_t next_test;

unsigned caption_test_send_next(void)
{
	unsigned sent = caption_out_send(obs_module_text(test_caption_keys[next_test]), TEST_DISPLAY_S);
	obs_log(LOG_INFO, "test caption %zu sent (flags %u)", next_test + 1, sent);
	next_test = (next_test + 1) % TEST_CAPTION_COUNT;
	return sent;
}

unsigned caption_test_clear(void)
{
	unsigned sent = caption_out_send("", 1.0);
	obs_log(LOG_INFO, "clear caption sent (flags %u)", sent);
	return sent;
}

const char *caption_test_result_key(unsigned sent_flags)
{
	const bool stream = sent_flags & CAPTION_SENT_STREAM;
	const bool recording = sent_flags & CAPTION_SENT_RECORDING;
	if (stream && recording)
		return "Status.StreamAndRecording";
	if (stream)
		return "Status.Stream";
	if (recording)
		return "Status.Recording";
	return "Status.PreviewOnly";
}
