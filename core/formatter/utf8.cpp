/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "formatter/utf8.h"

namespace slc::utf8 {

namespace {

constexpr char32_t replacement = 0xFFFD;

bool is_continuation(unsigned char b)
{
	return (b & 0xC0) == 0x80;
}

} // namespace

std::u32string decode(std::string_view text)
{
	std::u32string out;
	out.reserve(text.size());
	size_t i = 0;
	while (i < text.size()) {
		unsigned char b = static_cast<unsigned char>(text[i]);
		size_t len = 0;
		char32_t c = 0;
		if (b < 0x80) {
			len = 1;
			c = b;
		} else if ((b & 0xE0) == 0xC0) {
			len = 2;
			c = b & 0x1F;
		} else if ((b & 0xF0) == 0xE0) {
			len = 3;
			c = b & 0x0F;
		} else if ((b & 0xF8) == 0xF0) {
			len = 4;
			c = b & 0x07;
		}

		bool ok = len > 0 && i + len <= text.size();
		for (size_t k = 1; ok && k < len; k++) {
			unsigned char cb = static_cast<unsigned char>(text[i + k]);
			ok = is_continuation(cb);
			c = (c << 6) | (cb & 0x3F);
		}
		if (!ok) {
			out.push_back(replacement);
			i++;
			continue;
		}
		out.push_back(c);
		i += len;
	}
	return out;
}

std::string encode(char32_t c)
{
	std::string out;
	if (c < 0x80) {
		out += static_cast<char>(c);
	} else if (c < 0x800) {
		out += static_cast<char>(0xC0 | (c >> 6));
		out += static_cast<char>(0x80 | (c & 0x3F));
	} else if (c < 0x10000) {
		out += static_cast<char>(0xE0 | (c >> 12));
		out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
		out += static_cast<char>(0x80 | (c & 0x3F));
	} else {
		out += static_cast<char>(0xF0 | (c >> 18));
		out += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
		out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
		out += static_cast<char>(0x80 | (c & 0x3F));
	}
	return out;
}

std::string encode(const std::u32string &text)
{
	std::string out;
	for (char32_t c : text)
		out += encode(c);
	return out;
}

size_t length(std::string_view text)
{
	size_t n = 0;
	for (char ch : text)
		if (!is_continuation(static_cast<unsigned char>(ch)))
			n++;
	return n;
}

} // namespace slc::utf8
