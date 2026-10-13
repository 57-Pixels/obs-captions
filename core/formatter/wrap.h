/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Word wrapping and screen layout. Widths count code points (one 608 cell
 * each). Words are split on spaces; a word longer than a row is split hard.
 *
 * Phrase boundaries: when a line must break, it breaks after the last word
 * ending in , . ; : ? or ! instead, as long as the line stays at least two
 * thirds full and the moved words plus the next word fit on the next line.
 */

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace obscap {

constexpr size_t caption_max_bytes = 128; /* libobs CAPTION_LINE_BYTES */

std::vector<std::string> wrap_paragraph(std::string_view text, int cols);

/*
 * Wrap each paragraph (paragraphs always start on a new line), keep the
 * last rows lines, then drop the oldest lines until the result joined with
 * '\n' fits in max_bytes.
 */
std::string layout(const std::vector<std::string> &paragraphs, int cols, int rows,
		   size_t max_bytes = caption_max_bytes);

} // namespace obscap
