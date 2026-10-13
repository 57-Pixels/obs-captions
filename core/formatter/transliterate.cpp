/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "formatter/transliterate.h"

#include "formatter/charset-608.h"
#include "formatter/utf8.h"

namespace slc {

namespace {

/* Smart punctuation replaced by ASCII even when 608 has a glyph, so text
 * looks the same in every player. */
const char *ascii_punctuation(char32_t c)
{
	switch (c) {
	case 0x2018: /* left single quotation mark */
	case 0x2019: /* right single quotation mark */
	case 0x201A: /* single low-9 quotation mark */
	case 0x201B: /* single high-reversed-9 quotation mark */
	case 0x2032: /* prime */
		return "'";
	case 0x201C: /* left double quotation mark */
	case 0x201D: /* right double quotation mark */
	case 0x201E: /* double low-9 quotation mark */
	case 0x2033: /* double prime */
		return "\"";
	case 0x2013: /* en dash */
	case 0x2014: /* em dash */
	case 0x2015: /* horizontal bar */
	case 0x2212: /* minus sign */
		return "--";
	case 0x2026: /* horizontal ellipsis */
		return "...";
	default:
		return nullptr;
	}
}

bool is_space(char32_t c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f' || c == 0x00A0 ||
	       c == 0x1680 || (c >= 0x2000 && c <= 0x200A) || c == 0x2028 || c == 0x2029 || c == 0x202F ||
	       c == 0x205F || c == 0x3000;
}

/* Characters that take no space on screen: drop without counting. */
bool is_invisible(char32_t c)
{
	return c < 0x20 || c == 0x7F || (c >= 0x80 && c < 0xA0) || (c >= 0x0300 && c <= 0x036F) ||
	       (c >= 0x200B && c <= 0x200F) || c == 0x2060 || c == 0xFEFF || (c >= 0xFE00 && c <= 0xFE0F);
}

} // namespace

Transliterated transliterate_608(std::string_view text)
{
	Transliterated result;
	bool pending_space = false;

	auto append = [&](const std::string &s) {
		if (pending_space && !result.text.empty())
			result.text += ' ';
		pending_space = false;
		result.text += s;
	};

	for (char32_t c : utf8::decode(text)) {
		if (is_space(c)) {
			pending_space = true;
		} else if (const char *p = ascii_punctuation(c)) {
			append(p);
		} else if (is_608_char(c)) {
			append(utf8::encode(c));
		} else if (const char *f = fold_to_ascii(c)) {
			append(f);
		} else if (!is_invisible(c)) {
			result.stripped++;
		}
	}
	return result;
}

} // namespace slc
