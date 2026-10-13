/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Make arbitrary UTF-8 text displayable as CEA-608:
 * 1. Smart quotes become ASCII quotes, en/em dashes "--", ellipsis "...".
 * 2. Characters in the 608 set are kept (accented letters included).
 * 3. Other Latin letters fall back to their base letter (U+0142 -> "l").
 * 4. Any run of whitespace becomes one space; leading/trailing spaces go.
 *    Zero-width characters, combining marks and control characters are
 *    dropped silently.
 * 5. Everything else (emoji, CJK, symbols) is removed and counted.
 */

#include <cstddef>
#include <string>
#include <string_view>

namespace slc {

struct Transliterated {
	std::string text;
	size_t stripped = 0; /* visible characters that were removed */
};

Transliterated transliterate_608(std::string_view text);

} // namespace slc
