/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <doctest/doctest.h>

#include <cstring>
#include <string>

#include "slc/c-api.h"

TEST_CASE("C API: final hypothesis becomes a caption update")
{
	slc_pipeline *p = slc_pipeline_create(0);
	REQUIRE(p != nullptr);

	slc_pipeline_push(p, 1, "hello chat", 0, 800, 0);
	char text[SLC_CAPTION_MAX_BYTES + 1];
	int64_t duration_ms = 0;
	REQUIRE(slc_pipeline_tick(p, 800, text, sizeof(text), &duration_ms) == 1);
	CHECK(std::string(text) == "hello chat");
	CHECK(duration_ms == 1000);

	/* Unchanged: no update. */
	CHECK(slc_pipeline_tick(p, 2000, text, sizeof(text), &duration_ms) == 0);
	/* Silence: clear (empty text). */
	REQUIRE(slc_pipeline_tick(p, 4800, text, sizeof(text), &duration_ms) == 1);
	CHECK(std::strlen(text) == 0);

	slc_pipeline_destroy(p);
}

TEST_CASE("C API: stable partials are shown when enabled")
{
	slc_pipeline *p = slc_pipeline_create(1);
	slc_pipeline_push(p, 0, "we need to push", 0, 500, 10);
	char text[SLC_CAPTION_MAX_BYTES + 1];
	int64_t duration_ms = 0;
	REQUIRE(slc_pipeline_tick(p, 500, text, sizeof(text), &duration_ms) == 1);
	CHECK(std::string(text) == "we need to");
	slc_pipeline_destroy(p);
}

TEST_CASE("C API: reset forgets captions")
{
	slc_pipeline *p = slc_pipeline_create(0);
	slc_pipeline_push(p, 1, "one", 0, 100, 0);
	char text[SLC_CAPTION_MAX_BYTES + 1];
	int64_t duration_ms = 0;
	REQUIRE(slc_pipeline_tick(p, 100, text, sizeof(text), &duration_ms) == 1);
	slc_pipeline_reset(p);
	slc_pipeline_push(p, 1, "two", 200, 300, 0);
	REQUIRE(slc_pipeline_tick(p, 300, text, sizeof(text), &duration_ms) == 1);
	CHECK(std::string(text) == "two");
	slc_pipeline_destroy(p);
}
