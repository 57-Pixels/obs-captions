/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "protocol.h"

#include <cstdio>
#include <cstring>

namespace slc::engine {

namespace {

uint32_t read_u32(const uint8_t *p)
{
	return static_cast<uint32_t>(p[0]) | static_cast<uint32_t>(p[1]) << 8 | static_cast<uint32_t>(p[2]) << 16 |
	       static_cast<uint32_t>(p[3]) << 24;
}

uint64_t read_u64(const uint8_t *p)
{
	return static_cast<uint64_t>(read_u32(p)) | static_cast<uint64_t>(read_u32(p + 4)) << 32;
}

/* Hard limit on one frame (10 s of audio) so a corrupt length cannot make
 * the engine allocate without bound. */
constexpr uint32_t max_frame_samples = 160000;

} // namespace

void FrameReader::feed(const uint8_t *data, size_t size)
{
	if (pos > 0 && pos == buf.size()) {
		buf.clear();
		pos = 0;
	}
	buf.insert(buf.end(), data, data + size);
}

std::optional<FrameReader::Message> FrameReader::next()
{
	if (error || pos >= buf.size())
		return std::nullopt;

	const uint8_t *p = buf.data() + pos;
	const size_t avail = buf.size() - pos;
	Message msg;
	msg.type = p[0];

	if (msg.type == frame_json) {
		const void *nl = std::memchr(p + 1, '\n', avail - 1);
		if (!nl)
			return std::nullopt;
		size_t len = static_cast<size_t>(static_cast<const uint8_t *>(nl) - (p + 1));
		msg.json.assign(reinterpret_cast<const char *>(p + 1), len);
		pos += 1 + len + 1;
		return msg;
	}

	if (msg.type != frame_audio) {
		error = true;
		return std::nullopt;
	}
	if (avail < audio_header_size)
		return std::nullopt;
	if (std::memcmp(p + 1, audio_magic, sizeof(audio_magic)) != 0) {
		error = true;
		return std::nullopt;
	}
	uint32_t n = read_u32(p + 17);
	if (n > max_frame_samples) {
		error = true;
		return std::nullopt;
	}
	if (avail < audio_header_size + 2 * static_cast<size_t>(n))
		return std::nullopt;

	msg.audio.seq = read_u32(p + 5);
	msg.audio.clock_ns = read_u64(p + 9);
	msg.audio.samples.resize(n);
	const uint8_t *s = p + audio_header_size;
	for (uint32_t i = 0; i < n; i++) {
		int16_t v = static_cast<int16_t>(static_cast<uint16_t>(s[2 * i]) | static_cast<uint16_t>(s[2 * i + 1])
											   << 8);
		msg.audio.samples[i] = static_cast<float>(v) / 32768.0f;
	}
	pos += audio_header_size + 2 * static_cast<size_t>(n);
	return msg;
}

std::string json_string(std::string_view text)
{
	std::string out = "\"";
	for (char c : text) {
		switch (c) {
		case '"':
			out += "\\\"";
			break;
		case '\\':
			out += "\\\\";
			break;
		case '\n':
			out += "\\n";
			break;
		case '\r':
			out += "\\r";
			break;
		case '\t':
			out += "\\t";
			break;
		default:
			if (static_cast<unsigned char>(c) < 0x20) {
				char esc[8];
				std::snprintf(esc, sizeof(esc), "\\u%04x", static_cast<unsigned>(c));
				out += esc;
			} else {
				out += c;
			}
		}
	}
	return out + "\"";
}

std::string hello_message(const char *engine_version, const char *backend)
{
	return std::string("{\"type\":\"hello\",\"protocol\":0,\"engine_version\":") + json_string(engine_version) +
	       ",\"capabilities\":{\"backends\":[" + json_string(backend) +
	       "],\"partials\":true,\"stable_prefix\":true,\"word_list\":false,\"languages\":[\"en\"]}}";
}

std::string hypothesis_message(bool final, std::string_view text, double t_start_s, double t_end_s,
			       size_t stable_prefix_len)
{
	char times[96];
	std::snprintf(times, sizeof(times), ",\"t_start\":%.3f,\"t_end\":%.3f,\"stable_prefix_len\":%zu}", t_start_s,
		      t_end_s, stable_prefix_len);
	return std::string("{\"type\":\"hypothesis\",\"kind\":\"") + (final ? "final" : "partial") +
	       "\",\"text\":" + json_string(text) + times;
}

std::string status_message(const char *model, const char *state)
{
	return std::string("{\"type\":\"status\",\"model\":") + json_string(model) +
	       ",\"state\":" + json_string(state) + "}";
}

std::string error_message(const char *code, std::string_view message, const char *user_message_key)
{
	return std::string("{\"type\":\"error\",\"code\":") + json_string(code) +
	       ",\"message\":" + json_string(message) + ",\"user_message_key\":" + json_string(user_message_key) + "}";
}

std::string json_type(std::string_view line)
{
	/* Control lines are produced by our own plugin with a fixed layout, so
	 * a full JSON parser is not needed here. */
	const std::string_view key = "\"type\":\"";
	size_t at = line.find(key);
	if (at == std::string_view::npos)
		return {};
	size_t start = at + key.size();
	size_t end = line.find('"', start);
	if (end == std::string_view::npos)
		return {};
	return std::string(line.substr(start, end - start));
}

} // namespace slc::engine
