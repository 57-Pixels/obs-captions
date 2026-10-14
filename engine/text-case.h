/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * The GigaSpeech zipformer outputs ALL CAPS without punctuation. All-caps
 * captions read as shouting, so the engine converts to sentence case:
 * lowercase, first letter of the utterance capitalized, and "I" (also in
 * I'm, I'll, I'd, I've) capitalized. Words are not changed otherwise
 * ("keep speech as said", docs/OPEN_QUESTIONS.md 18).
 */

#include <string>
#include <string_view>

namespace slc::engine {

std::string sentence_case(std::string_view text);

} // namespace slc::engine
