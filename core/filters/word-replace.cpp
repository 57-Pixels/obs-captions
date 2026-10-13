/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "filters/word-replace.h"

#include "filters/text-norm.h"

namespace obscap {

namespace {

bool boundary_before(const std::string &text, size_t pos)
{
	return pos == 0 || !is_word_byte(text[pos - 1]);
}

bool boundary_after(const std::string &text, size_t end)
{
	return end >= text.size() || !is_word_byte(text[end]);
}

std::string replace_one(const std::string &text, const std::string &from, const std::string &to)
{
	if (from.empty())
		return text;
	const std::string lower_text = to_lower_ascii(text);
	const std::string lower_from = to_lower_ascii(from);

	std::string out;
	size_t pos = 0;
	while (true) {
		size_t hit = lower_text.find(lower_from, pos);
		if (hit == std::string::npos)
			break;
		size_t end = hit + lower_from.size();
		if (boundary_before(text, hit) && boundary_after(text, end)) {
			out.append(text, pos, hit - pos);
			out += to;
			pos = end;
		} else {
			out.append(text, pos, hit + 1 - pos);
			pos = hit + 1;
		}
	}
	out.append(text, pos, std::string::npos);
	return out;
}

} // namespace

std::string replace_words(std::string_view text, const std::vector<std::pair<std::string, std::string>> &rules)
{
	std::string out(text);
	for (const auto &[from, to] : rules)
		out = replace_one(out, from, to);
	return out;
}

} // namespace obscap
