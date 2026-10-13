/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Caption pipeline configuration. Every behaviour is configurable; the
 * defaults below are the "one switch" experience. Call sanitize() before use
 * (CaptionPipeline does this) so out-of-range values are clamped instead of
 * producing broken captions.
 */

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace obscap {

struct FormatterConfig {
	/* Characters per row. 28 reads well on phones; CEA-608 allows 32. */
	int cols = 28;
	/* Rows on screen (pop-on). */
	int rows = 2;
};

struct SchedulerConfig {
	/* Minimum time between two caption updates sent to OBS. */
	int64_t min_interval_ms = 1000;
	/* Clear the screen after this long without new text. */
	int64_t clear_after_ms = 4000;
	/* Also show the stable (non-retractable) prefix of the current partial. */
	bool show_stable_partial = false;
};

struct FilterConfig {
	/* Suppress the Nth consecutive (near-)duplicate final and beyond. */
	int repeat_threshold = 3;
	/* Finals further apart than this never count as a repeat run. */
	int64_t repeat_window_ms = 10000;
	/* Max normalized edit distance (0..1) for "near duplicate". */
	double near_duplicate_ratio = 0.2;

	/* Silence-artifact filter (see filters/hallucination.h for the rule). */
	bool hallucination_filter = true;
	int hallucination_max_words = 8;
	int64_t hallucination_isolation_ms = 2000;
	std::vector<std::string> hallucination_blocklist;

	/* Case-insensitive, whole-word replacements, applied first. */
	std::vector<std::pair<std::string, std::string>> word_replacements;

	bool profanity_mask = false;
	std::vector<std::string> profanity_words;

	FilterConfig();
};

struct Config {
	FormatterConfig formatter;
	SchedulerConfig scheduler;
	FilterConfig filters;
};

/* Limits enforced by sanitize(). */
constexpr int min_cols = 8;
constexpr int max_cols = 32; /* CEA-608 row width */
constexpr int min_rows = 1;
constexpr int max_rows = 4; /* 608 pop-on practice; also keeps text <= 128 B */
constexpr int64_t min_interval_floor_ms = 100;

/* Returns a copy with every value clamped to a usable range. */
Config sanitize(Config config);

std::vector<std::string> default_hallucination_blocklist();
std::vector<std::string> default_profanity_words();

} // namespace obscap
