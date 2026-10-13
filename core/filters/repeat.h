/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Repeat suppression, two parts:
 *
 * 1. Within one utterance: a phrase of 1-6 words repeated threshold or more
 *    times in a row is cut to threshold-1 copies (words compared after
 *    normalize()).
 *    "I mean I mean I mean I mean it works" -> "I mean I mean it works"
 *
 * 2. Across finals: consecutive accepted finals whose normalized text is
 *    equal or within near_duplicate_ratio edit distance form a run, as long
 *    as each starts within repeat_window_ms of the previous one's end. The
 *    threshold-th final of a run and every later one are suppressed.
 *
 * 3. Engine re-sends: a final with exactly the same normalized text as the
 *    previous one that starts before the previous one ended is a duplicate
 *    of the same speech. It is suppressed and does not extend the run.
 */

#include <cstdint>
#include <string>
#include <string_view>

#include "slc/config.h"

namespace slc {

std::string collapse_repeats(std::string_view text, int threshold);

class RepeatFilter {
public:
	explicit RepeatFilter(const FilterConfig &config);

	/* Records the final and returns true if it must be suppressed. */
	bool suppress(std::string_view text, int64_t t_start_ms, int64_t t_end_ms);
	void reset();

private:
	int threshold;
	int64_t window_ms;
	double ratio;
	std::string last_norm;
	int64_t last_end_ms = 0;
	int run = 0;
};

} // namespace slc
