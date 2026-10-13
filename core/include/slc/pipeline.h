/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * The caption pipeline: filters -> scheduler -> formatter.
 *
 * Usage (one thread only, the plugin's engine-client thread):
 *   push() every hypothesis or TTS line as it arrives, call tick(now) about
 *   every 100 ms, and hand any returned CaptionUpdate to caption-out.c.
 *
 * Deterministic: the pipeline never reads a clock. Time comes from the event
 * timestamps and the now_ms passed to tick().
 */

#include <memory>
#include <optional>

#include "slc/caption-update.h"
#include "slc/config.h"
#include "slc/events.h"
#include "slc/stats.h"

namespace slc {

class CaptionPipeline {
public:
	explicit CaptionPipeline(const Config &config = Config{});
	~CaptionPipeline();

	CaptionPipeline(const CaptionPipeline &) = delete;
	CaptionPipeline &operator=(const CaptionPipeline &) = delete;

	void push(const Hypothesis &hypothesis);
	void push(const TextEvent &text);

	/* Returns at most one update per minimum interval, only on change. */
	std::optional<CaptionUpdate> tick(int64_t now_ms);

	/* Forget all caption state (call on stream start). Stats are kept. */
	void reset();

	const Stats &stats() const;
	const Config &config() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

} // namespace slc
