/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "filters/text-norm.h"

#include <algorithm>
#include <vector>

namespace slc {

bool is_word_byte(char c)
{
	unsigned char u = static_cast<unsigned char>(c);
	return u >= 0x80 || (u >= '0' && u <= '9') || (u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z');
}

std::string to_lower_ascii(std::string_view text)
{
	std::string out(text);
	for (char &c : out)
		if (c >= 'A' && c <= 'Z')
			c = static_cast<char>(c - 'A' + 'a');
	return out;
}

std::string normalize(std::string_view text)
{
	std::string out;
	bool space = false;
	for (char c : to_lower_ascii(text)) {
		if (!is_word_byte(c)) {
			space = true;
			continue;
		}
		if (space && !out.empty())
			out += ' ';
		space = false;
		out += c;
	}
	return out;
}

size_t word_count(std::string_view normalized)
{
	if (normalized.empty())
		return 0;
	return static_cast<size_t>(std::count(normalized.begin(), normalized.end(), ' ')) + 1;
}

double edit_ratio(std::string_view a, std::string_view b)
{
	size_t longest = std::max(a.size(), b.size());
	if (longest == 0)
		return 0.0;

	/* Two-row Levenshtein; inputs are short caption phrases. */
	std::vector<size_t> prev(b.size() + 1), cur(b.size() + 1);
	for (size_t j = 0; j <= b.size(); j++)
		prev[j] = j;
	for (size_t i = 1; i <= a.size(); i++) {
		cur[0] = i;
		for (size_t j = 1; j <= b.size(); j++) {
			size_t cost = a[i - 1] == b[j - 1] ? 0 : 1;
			cur[j] = std::min({prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + cost});
		}
		std::swap(prev, cur);
	}
	return static_cast<double>(prev[b.size()]) / static_cast<double>(longest);
}

} // namespace slc
