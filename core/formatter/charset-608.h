/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * The characters OBS can put on screen. OBS encodes caption text with its
 * bundled libcaption; the tables in charset-608.cpp mirror
 * deps/libcaption/src/eia608_from_utf8.re2c in OBS 30.2.3 exactly.
 */

namespace slc {

/* True for printable ASCII and the CEA-608 special/extended characters. */
bool is_608_char(char32_t c);

/* ASCII replacement for a Latin letter 608 cannot show ("L" for U+0141,
 * "ae" for U+00E6), or nullptr if there is none. */
const char *fold_to_ascii(char32_t c);

} // namespace slc
