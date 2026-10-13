/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "formatter/charset-608.h"

#include <algorithm>
#include <iterator>

#include "formatter/charset-608-tables.h"

namespace obscap {

bool is_608_char(char32_t c)
{
	/* libcaption maps every printable ASCII character, remapping the few
	 * that 608's basic set lacks (eia608_from_utf8.re2c, "Ascii Exceptions"). */
	if (c >= 0x20 && c < 0x7F)
		return true;
	return std::binary_search(std::begin(tables::extended_608), std::end(tables::extended_608), c);
}

const char *fold_to_ascii(char32_t c)
{
	auto it = std::lower_bound(std::begin(tables::fold_ascii), std::end(tables::fold_ascii), c,
				   [](const tables::Fold &f, char32_t v) { return f.from < v; });
	if (it == std::end(tables::fold_ascii) || it->from != c)
		return nullptr;
	return it->to;
}

} // namespace obscap
