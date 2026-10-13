/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Optional profanity mask (default off): listed words, case-insensitive and
 * whole-word, keep their first letter and the rest becomes '*'.
 * "shit" -> "s***".
 */

#include <string>
#include <string_view>
#include <vector>

namespace slc {

std::string mask_profanity(std::string_view text, const std::vector<std::string> &words);

} // namespace slc
