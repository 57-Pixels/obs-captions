/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * User word list: case-insensitive (ASCII), whole-word or whole-phrase
 * replacement. A match must not touch a letter or digit on either side.
 * Rules apply in order; replacements are inserted exactly as written.
 */

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace obscap {

std::string replace_words(std::string_view text, const std::vector<std::pair<std::string, std::string>> &rules);

} // namespace obscap
