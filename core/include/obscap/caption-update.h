/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <cstdint>
#include <string>

namespace obscap {

/*
 * One caption update for plugin/src/caption-out.c. text holds 1..rows lines
 * separated by '\n', only characters libcaption can encode as CEA-608, and
 * at most 128 bytes (libobs CAPTION_LINE_BYTES). An empty text means "clear
 * the screen". display_duration_ms is passed to
 * obs_output_output_caption_text2 and never exceeds the scheduler's minimum
 * interval, so the libobs caption FIFO never backs up (ADR 0003).
 */
struct CaptionUpdate {
	std::string text;
	int64_t display_duration_ms = 0;
};

} // namespace obscap
