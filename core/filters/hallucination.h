/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Silence-artifact ("hallucination") filter.
 *
 * A final is a CANDIDATE when its whole normalized text equals a blocklist
 * entry ("Thank you." -> "thank you") and it has at most max_words words.
 * Text that merely contains a blocklisted phrase ("I told you so") is never
 * a candidate.
 *
 * A candidate is suppressed only if it is ISOLATED: no accepted speech ended
 * within isolation_ms before it started, and no other speech starts within
 * isolation_ms after it ended. To decide the second part the candidate is
 * held back; it is released (accepted) as soon as other speech arrives in
 * time, or dropped once now >= t_end + isolation_ms.
 */

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "slc/config.h"
#include "slc/events.h"

namespace slc {

class HallucinationFilter {
public:
	explicit HallucinationFilter(const FilterConfig &config);

	bool is_candidate(std::string_view text) const;

	/*
	 * Feed a final. last_speech_end_ms is the end of the last accepted
	 * speech (use a very negative value for "none"). Returns the finals to
	 * accept now, in order: possibly a released earlier candidate, then this
	 * final unless it is held. *dropped is set if an earlier candidate was
	 * dropped instead of released.
	 */
	std::vector<Hypothesis> on_final(const Hypothesis &final_hyp, int64_t last_speech_end_ms, bool *dropped);

	/* A partial with real speech releases a held candidate. */
	std::optional<Hypothesis> on_partial(const Hypothesis &partial);

	/* Drops an expired candidate; returns true if one was dropped. */
	bool on_tick(int64_t now_ms);

	void reset();

private:
	bool releases(int64_t speech_start_ms) const;

	bool enabled;
	int max_words;
	int64_t isolation_ms;
	std::vector<std::string> blocklist; /* normalized */
	std::optional<Hypothesis> held;
};

} // namespace slc
