/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/* Text comparison helpers shared by the filters. */

#include <cstddef>
#include <string>
#include <string_view>

namespace obscap {

/*
 * Lowercase ASCII, turn every ASCII character that is not a letter or digit
 * into a space, collapse spaces, trim. Non-ASCII bytes are kept as-is.
 * "Thanks for watching!" -> "thanks for watching", "amara.org" -> "amara org".
 */
std::string normalize(std::string_view text);

size_t word_count(std::string_view normalized);

/* Levenshtein distance divided by the longer length (0 = equal). */
double edit_ratio(std::string_view a, std::string_view b);

/* True for ASCII letters/digits and any non-ASCII byte. */
bool is_word_byte(char c);

/* ASCII-only lowercase copy. */
std::string to_lower_ascii(std::string_view text);

} // namespace obscap
