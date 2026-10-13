/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/* Minimal UTF-8 helpers. Invalid bytes decode to U+FFFD. */

#include <cstddef>
#include <string>
#include <string_view>

namespace slc::utf8 {

std::u32string decode(std::string_view text);
std::string encode(char32_t c);
std::string encode(const std::u32string &text);
/* Number of code points. */
size_t length(std::string_view text);

} // namespace slc::utf8
