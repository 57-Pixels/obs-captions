/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "slc/c-api.h"

#include <cstring>

#include "slc/pipeline.h"

struct slc_pipeline {
	explicit slc_pipeline(const slc::Config &config) : pipeline(config) {}
	slc::CaptionPipeline pipeline;
};

slc_pipeline *slc_pipeline_create(int show_stable_partial)
{
	slc::Config config;
	config.scheduler.show_stable_partial = show_stable_partial != 0;
	return new slc_pipeline(config);
}

void slc_pipeline_destroy(slc_pipeline *pipeline)
{
	delete pipeline;
}

void slc_pipeline_push(slc_pipeline *pipeline, int is_final, const char *text, int64_t t_start_ms, int64_t t_end_ms,
		       size_t stable_prefix_len)
{
	slc::Hypothesis h;
	h.kind = is_final ? slc::HypothesisKind::final : slc::HypothesisKind::partial;
	h.text = text ? text : "";
	h.t_start_ms = t_start_ms;
	h.t_end_ms = t_end_ms;
	h.stable_prefix_len = stable_prefix_len;
	pipeline->pipeline.push(h);
}

int slc_pipeline_tick(slc_pipeline *pipeline, int64_t now_ms, char *text, size_t text_size,
		      int64_t *display_duration_ms)
{
	auto update = pipeline->pipeline.tick(now_ms);
	if (!update || text_size == 0)
		return 0;
	size_t n = update->text.size() < text_size - 1 ? update->text.size() : text_size - 1;
	std::memcpy(text, update->text.data(), n);
	text[n] = '\0';
	if (display_duration_ms)
		*display_duration_ms = update->display_duration_ms;
	return 1;
}

void slc_pipeline_reset(slc_pipeline *pipeline)
{
	pipeline->pipeline.reset();
}
