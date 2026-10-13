/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "filters/profanity.h"

#include <algorithm>

#include "filters/text-norm.h"

namespace slc {

std::string mask_profanity(std::string_view text, const std::vector<std::string> &words)
{
	std::string out(text);
	size_t i = 0;
	while (i < out.size()) {
		if (!is_word_byte(out[i])) {
			i++;
			continue;
		}
		size_t end = i;
		while (end < out.size() && is_word_byte(out[end]))
			end++;
		std::string word = to_lower_ascii(std::string_view(out).substr(i, end - i));
		if (std::find(words.begin(), words.end(), word) != words.end())
			for (size_t k = i + 1; k < end; k++)
				out[k] = '*';
		i = end;
	}
	return out;
}

} // namespace slc
