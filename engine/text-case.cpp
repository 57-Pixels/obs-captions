/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "text-case.h"

namespace slc::engine {

namespace {

char lower(char c)
{
	return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c;
}

char upper(char c)
{
	return c >= 'a' && c <= 'z' ? static_cast<char>(c - 'a' + 'A') : c;
}

bool is_letter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

} // namespace

std::string sentence_case(std::string_view text)
{
	std::string out;
	out.reserve(text.size());
	for (char c : text)
		out += lower(c);

	/* Standalone "i", or "i" followed by an apostrophe: I, I'm, I'll. */
	for (size_t i = 0; i < out.size(); i++) {
		if (out[i] != 'i')
			continue;
		bool start = i == 0 || !is_letter(out[i - 1]);
		bool end = i + 1 == out.size() || out[i + 1] == ' ' || out[i + 1] == '\'';
		if (start && end)
			out[i] = 'I';
	}

	for (char &c : out) {
		if (is_letter(c)) {
			c = upper(c);
			break;
		}
	}
	return out;
}

} // namespace slc::engine
