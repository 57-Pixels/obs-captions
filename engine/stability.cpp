/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "stability.h"

#include <algorithm>

namespace slc::engine {

size_t StabilityTracker::update(std::string_view text)
{
	history.emplace_back(text);
	while (history.size() > required)
		history.pop_front();
	if (history.size() < required)
		return 0;

	/* Longest common prefix of the recent partials. */
	size_t common = history.front().size();
	for (const auto &h : history) {
		common = std::min(common, h.size());
		for (size_t i = 0; i < common; i++) {
			if (h[i] != history.front()[i]) {
				common = i;
				break;
			}
		}
	}

	/* Cut back to a word boundary in the current text. */
	const std::string &cur = history.back();
	if (common >= cur.size())
		return cur.size();
	if (cur[common] == ' ')
		return common;
	size_t space = cur.rfind(' ', common);
	return space == std::string::npos ? 0 : space;
}

} // namespace slc::engine
