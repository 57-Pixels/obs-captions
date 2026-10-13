/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Input events. All times are milliseconds on one clock shared with
 * CaptionPipeline::tick(now_ms) (the plugin uses the OBS audio clock).
 */

#include <cstddef>
#include <cstdint>
#include <string>

namespace slc {

enum class HypothesisKind { partial, final };

/* A speech-recognition result (protocol "hypothesis" event). */
struct Hypothesis {
	HypothesisKind kind = HypothesisKind::partial;
	std::string text; /* UTF-8 */
	int64_t t_start_ms = 0;
	int64_t t_end_ms = 0;
	/* Bytes at the start of text that will not change. Ignored for finals. */
	size_t stable_prefix_len = 0;
	std::string speaker; /* optional; empty = unknown */
};

/* An exact line from a TTS or VTuber app. Already final; bypasses ASR. */
struct TextEvent {
	std::string text; /* UTF-8 */
	int64_t start_ms = 0;
	int64_t est_duration_ms = 0;
	std::string speaker; /* optional */
};

} // namespace slc
