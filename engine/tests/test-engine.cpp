/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <doctest/doctest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "protocol.h"
#include "stability.h"
#include "text-case.h"

using namespace slc::engine;

namespace {

std::vector<uint8_t> audio_frame(uint32_t seq, const std::vector<int16_t> &samples)
{
	std::vector<uint8_t> f = {frame_audio, 'O', 'C', 'A', 'F'};
	for (int i = 0; i < 4; i++)
		f.push_back(static_cast<uint8_t>(seq >> (8 * i)));
	for (int i = 0; i < 8; i++)
		f.push_back(0); /* clock_ns */
	uint32_t n = static_cast<uint32_t>(samples.size());
	for (int i = 0; i < 4; i++)
		f.push_back(static_cast<uint8_t>(n >> (8 * i)));
	for (int16_t s : samples) {
		f.push_back(static_cast<uint8_t>(static_cast<uint16_t>(s) & 0xFF));
		f.push_back(static_cast<uint8_t>(static_cast<uint16_t>(s) >> 8));
	}
	return f;
}

} // namespace

TEST_CASE("frame reader parses audio and JSON split across reads")
{
	std::vector<uint8_t> bytes = audio_frame(7, {0, 16384, -32768});
	std::string json = "\x02{\"type\":\"shutdown\"}\n";
	bytes.insert(bytes.end(), json.begin(), json.end());

	FrameReader r;
	/* Feed one byte at a time: messages must only appear when complete. */
	std::vector<FrameReader::Message> got;
	for (uint8_t b : bytes) {
		r.feed(&b, 1);
		while (auto m = r.next())
			got.push_back(*m);
	}
	REQUIRE(got.size() == 2);
	CHECK(got[0].type == frame_audio);
	CHECK(got[0].audio.seq == 7);
	REQUIRE(got[0].audio.samples.size() == 3);
	CHECK(got[0].audio.samples[1] == doctest::Approx(0.5));
	CHECK(got[0].audio.samples[2] == doctest::Approx(-1.0));
	CHECK(got[1].type == frame_json);
	CHECK(json_type(got[1].json) == "shutdown");
	CHECK_FALSE(r.broken());
}

TEST_CASE("frame reader rejects corrupt input")
{
	FrameReader r;
	uint8_t junk[] = {0x7F, 1, 2, 3};
	r.feed(junk, sizeof(junk));
	CHECK_FALSE(r.next().has_value());
	CHECK(r.broken());
}

TEST_CASE("JSON messages escape text")
{
	CHECK(json_string("a\"b\\c\nd") == "\"a\\\"b\\\\c\\nd\"");
	std::string h = hypothesis_message(true, "say \"hi\"", 1.5, 2.25, 8);
	CHECK(h == "{\"type\":\"hypothesis\",\"kind\":\"final\",\"text\":\"say \\\"hi\\\"\",\"t_start\":1.500,"
		   "\"t_end\":2.250,\"stable_prefix_len\":8}");
	CHECK(json_type("{\"type\":\"hello_ack\",\"protocol\":0}") == "hello_ack");
	CHECK(json_type("{}") == "");
}

TEST_CASE("sentence case keeps words, fixes shouting")
{
	CHECK(sentence_case("WUZ UP CHAT") == "Wuz up chat");
	CHECK(sentence_case("I THINK I'M GONNA PUSH") == "I think I'm gonna push");
	CHECK(sentence_case("WHAT DID I DO") == "What did I do");
	CHECK(sentence_case("IT IS MINE") == "It is mine"); /* "i" inside words untouched */
	CHECK(sentence_case("") == "");
}

TEST_CASE("stability needs three matching partials and ends on a word")
{
	StabilityTracker s;
	CHECK(s.update("we need") == 0);
	CHECK(s.update("we need to") == 0);
	/* "we need" common to all three; cut at the word boundary. */
	CHECK(s.update("we need to push") == 7);
	CHECK(s.update("we need to push mid") == 10);
	/* A revision of an earlier word shrinks the stable part. */
	CHECK(s.update("we need two push mid") == 7);
	s.reset();
	CHECK(s.update("hello") == 0);
}
