/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Estimates which part of a streaming partial will not change, for the
 * protocol's stable_prefix_len. A word counts as stable once it has been
 * identical (same position) in the last `required` partial results. The
 * stable prefix always ends at a word boundary.
 *
 * Streaming transducers mostly revise only the last word or two, so this is
 * a simple, model-independent rule.
 */

#include <cstddef>
#include <deque>
#include <string>
#include <string_view>

namespace slc::engine {

class StabilityTracker {
public:
	explicit StabilityTracker(size_t required = 3) : required(required) {}

	/* Feed the next partial text; returns its stable prefix length in bytes. */
	size_t update(std::string_view text);
	void reset() { history.clear(); }

private:
	size_t required;
	std::deque<std::string> history;
};

} // namespace slc::engine
