/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "slc/config.h"

#include <algorithm>

#include "slc/stats.h"

namespace slc {

std::vector<std::string> default_hallucination_blocklist()
{
	/* Known silence/music artifacts of speech models (mostly Whisper-style
	 * training data: video outros and subtitle credits). Matched against
	 * the WHOLE normalized utterance only; see filters/hallucination.h. */
	return {
		"you",
		"thank you",
		"thank you so much",
		"thank you very much",
		"thanks for watching",
		"thank you for watching",
		"thanks for watching bye",
		"thank you for watching bye",
		"please subscribe",
		"like and subscribe",
		"subtitles by the amara org community",
		"subtitles by amara org",
	};
}

std::vector<std::string> default_profanity_words()
{
	return {
		"fuck",   "fucking",  "fucked", "fucker",  "motherfucker", "shit",
		"shitty", "bullshit", "bitch",  "bitches", "cunt",         "asshole",
	};
}

FilterConfig::FilterConfig()
	: hallucination_blocklist(default_hallucination_blocklist()),
	  profanity_words(default_profanity_words())
{
}

Config sanitize(Config c)
{
	c.formatter.cols = std::clamp(c.formatter.cols, min_cols, max_cols);
	c.formatter.rows = std::clamp(c.formatter.rows, min_rows, max_rows);

	c.scheduler.min_interval_ms = std::max(c.scheduler.min_interval_ms, min_interval_floor_ms);
	c.scheduler.clear_after_ms = std::max(c.scheduler.clear_after_ms, c.scheduler.min_interval_ms);

	c.filters.repeat_threshold = std::max(c.filters.repeat_threshold, 2);
	c.filters.repeat_window_ms = std::max<int64_t>(c.filters.repeat_window_ms, 0);
	c.filters.near_duplicate_ratio = std::clamp(c.filters.near_duplicate_ratio, 0.0, 1.0);
	c.filters.hallucination_max_words = std::max(c.filters.hallucination_max_words, 1);
	c.filters.hallucination_isolation_ms = std::max<int64_t>(c.filters.hallucination_isolation_ms, 0);
	return c;
}

const char *suppress_reason_name(SuppressReason reason)
{
	switch (reason) {
	case SuppressReason::repeat:
		return "repeat";
	case SuppressReason::hallucination:
		return "hallucination";
	case SuppressReason::superseded:
		return "superseded";
	case SuppressReason::count:
		break;
	}
	return "unknown";
}

} // namespace slc
