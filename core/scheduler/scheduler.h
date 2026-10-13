/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Latest-wins caption scheduler (ADR 0003).
 *
 * Holds the text on screen as segments: committed finals and TTS lines,
 * plus optionally the stable part of the current partial. tick(now):
 *
 * 1. If text is on screen and now >= newest segment end + clear_after_ms,
 *    the segments are forgotten and the next update clears the screen.
 * 2. The display text is laid out (rolling window of the last rows lines).
 * 3. If it differs from what was last sent and min_interval_ms has passed
 *    since the last update, it is returned with display_duration =
 *    min_interval_ms. Otherwise it waits; if it changes again before being
 *    sent, the waiting text counts as "superseded". Nothing is queued.
 *
 * Speaker changes: a segment whose known speaker differs from the previous
 * known speaker starts a new line with ">> " (CEA-608 convention).
 */

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "slc/caption-update.h"
#include "slc/config.h"
#include "slc/stats.h"

namespace slc {

/* The stable prefix of a partial, cut back to whole words. */
std::string stable_words(std::string_view text, size_t stable_prefix_len);

class Scheduler {
public:
	/*
	 * Committed text is kept as at most max_segments segments. When that is
	 * exceeded, the oldest trim_batch segments are dropped at once (they are
	 * long off screen), so the remaining text re-wraps rarely, not on every
	 * new final.
	 */
	static constexpr size_t max_segments = 64;
	static constexpr size_t trim_batch = 16;

	Scheduler(const FormatterConfig &formatter, const SchedulerConfig &scheduler);

	/* text must already be filtered and transliterated. */
	void commit(const std::string &text, const std::string &speaker, int64_t t_end_ms);
	/* Replace the partial shown after the committed text ("" = none). */
	void set_partial(const std::string &text, const std::string &speaker, int64_t t_end_ms);

	std::optional<CaptionUpdate> tick(int64_t now_ms, Stats &stats);
	void reset();

	size_t segment_count() const { return segments.size(); }

private:
	struct Segment {
		std::string text;
		bool new_speaker = false;
		int64_t t_end_ms = 0;
	};

	bool is_new_speaker(const std::string &speaker) const;
	int64_t newest_end_ms() const;
	std::string render() const;

	FormatterConfig fmt;
	SchedulerConfig cfg;
	std::vector<Segment> segments;
	std::optional<Segment> partial;
	std::string last_speaker;
	std::string last_sent;
	std::optional<std::string> waiting;
	std::optional<int64_t> last_sent_ms;
	int64_t sent_newest_end_ms = 0;
};

} // namespace slc
