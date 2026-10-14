/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Engine side of protocol/PROTOCOL.md v0: frame parsing (plugin -> engine)
 * and JSON-line messages (engine -> plugin). Pure functions, no I/O, so they
 * are unit-tested in engine/tests.
 */

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace slc::engine {

constexpr uint8_t frame_audio = 0x01;
constexpr uint8_t frame_json = 0x02;
constexpr char audio_magic[4] = {'O', 'C', 'A', 'F'};
/* type(1) + magic(4) + seq(4) + clock_ns(8) + n_samples(4) */
constexpr size_t audio_header_size = 21;

struct AudioFrame {
	uint32_t seq = 0;
	uint64_t clock_ns = 0;
	std::vector<float> samples; /* 16 kHz mono, -1..1 */
};

/*
 * Incremental parser for the plugin -> engine byte stream. Feed bytes as
 * they arrive; complete audio frames and JSON lines come out in order.
 */
class FrameReader {
public:
	struct Message {
		uint8_t type = 0;
		AudioFrame audio; /* type == frame_audio */
		std::string json; /* type == frame_json, without '\n' */
	};

	void feed(const uint8_t *data, size_t size);
	std::optional<Message> next();
	/* Set when the stream is corrupt (bad magic or unknown type). */
	bool broken() const { return error; }

private:
	std::vector<uint8_t> buf;
	size_t pos = 0;
	bool error = false;
};

/* JSON string literal with quotes and escapes. */
std::string json_string(std::string_view text);

std::string hello_message(const char *engine_version, const char *backend);
std::string hypothesis_message(bool final, std::string_view text, double t_start_s, double t_end_s,
			       size_t stable_prefix_len);
std::string status_message(const char *model, const char *state);
std::string error_message(const char *code, std::string_view message, const char *user_message_key);

/* "type" of a JSON control line, e.g. "shutdown"; empty if not found. */
std::string json_type(std::string_view line);

} // namespace slc::engine
