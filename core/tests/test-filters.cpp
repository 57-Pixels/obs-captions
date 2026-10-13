/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <doctest/doctest.h>

#include <string>
#include <vector>

#include "filters/hallucination.h"
#include "filters/profanity.h"
#include "filters/repeat.h"
#include "filters/text-norm.h"
#include "filters/word-replace.h"

using namespace slc;

namespace {

Hypothesis final_hyp(const std::string &text, int64_t start, int64_t end)
{
	Hypothesis h;
	h.kind = HypothesisKind::final;
	h.text = text;
	h.t_start_ms = start;
	h.t_end_ms = end;
	return h;
}

constexpr int64_t no_speech = -1000000;

} // namespace

TEST_CASE("normalize")
{
	CHECK(normalize("  Thanks for WATCHING!! ") == "thanks for watching");
	CHECK(normalize("amara.org") == "amara org");
	CHECK(normalize("...") == "");
	CHECK(word_count("thank you") == 2);
	CHECK(word_count("") == 0);
	CHECK(edit_ratio("abc", "abc") == 0.0);
	CHECK(edit_ratio("lets go", "let s go") == doctest::Approx(1.0 / 8.0));
}

TEST_CASE("word replacement is case-insensitive and whole-word")
{
	std::vector<std::pair<std::string, std::string>> rules{{"valerant", "Valorant"}, {"gee gee", "GG"}};
	CHECK(replace_words("valerant and VALERANT", rules) == "Valorant and Valorant");
	CHECK(replace_words("valerants", rules) == "valerants");
	CHECK(replace_words("xvalerant", rules) == "xvalerant");
	CHECK(replace_words("Gee Gee, team", rules) == "GG, team");
	CHECK(replace_words("nothing here", {}) == "nothing here");
}

TEST_CASE("profanity mask")
{
	std::vector<std::string> words{"shit", "fuck"};
	CHECK(mask_profanity("oh SHIT that hurt", words) == "oh S*** that hurt");
	CHECK(mask_profanity("shitake", words) == "shitake");
	CHECK(mask_profanity("clean", words) == "clean");
}

TEST_CASE("repeats inside one utterance collapse")
{
	CHECK(collapse_repeats("no no no no", 3) == "no no");
	CHECK(collapse_repeats("I mean I mean I mean I mean it works", 3) == "I mean I mean it works");
	CHECK(collapse_repeats("no no", 3) == "no no");
	CHECK(collapse_repeats("go Go GO!", 3) == "go Go");
	CHECK(collapse_repeats("a b c", 3) == "a b c");
}

TEST_CASE("repeat filter suppresses the third near-duplicate final")
{
	FilterConfig cfg;
	RepeatFilter f(cfg);
	CHECK_FALSE(f.suppress("gg", 0, 500));
	CHECK_FALSE(f.suppress("GG!", 1000, 1500));
	CHECK(f.suppress("gg", 2000, 2500));
	CHECK(f.suppress("gg", 3000, 3500));
	CHECK_FALSE(f.suppress("something else", 4000, 4500));
	CHECK_FALSE(f.suppress("gg", 5000, 5500));
}

TEST_CASE("repeat run resets after the window")
{
	FilterConfig cfg;
	RepeatFilter f(cfg);
	CHECK_FALSE(f.suppress("gg", 0, 500));
	CHECK_FALSE(f.suppress("gg", 1000, 1500));
	CHECK_FALSE(f.suppress("gg", 1500 + cfg.repeat_window_ms + 1, 20000));
}

TEST_CASE("hallucination candidates are whole short blocklist matches")
{
	FilterConfig cfg;
	HallucinationFilter f(cfg);
	CHECK(f.is_candidate("you"));
	CHECK(f.is_candidate("Thank you."));
	CHECK(f.is_candidate("Thanks for watching!"));
	CHECK(f.is_candidate("Subtitles by the Amara.org community"));
	CHECK_FALSE(f.is_candidate("I told you so"));
	CHECK_FALSE(f.is_candidate("thank you for the raid"));

	cfg.hallucination_max_words = 2;
	HallucinationFilter short_only(cfg);
	CHECK(short_only.is_candidate("thank you"));
	CHECK_FALSE(short_only.is_candidate("thanks for watching"));

	cfg.hallucination_filter = false;
	HallucinationFilter off(cfg);
	CHECK_FALSE(off.is_candidate("you"));
}

TEST_CASE("isolated candidate is held then dropped")
{
	FilterConfig cfg;
	HallucinationFilter f(cfg);
	bool dropped = false;
	CHECK(f.on_final(final_hyp("you", 10000, 10300), no_speech, &dropped).empty());
	CHECK_FALSE(dropped);
	CHECK_FALSE(f.on_tick(12299));
	CHECK(f.on_tick(12300));
	CHECK_FALSE(f.on_tick(12400));
}

TEST_CASE("candidate after recent speech is accepted at once")
{
	FilterConfig cfg;
	HallucinationFilter f(cfg);
	bool dropped = false;
	auto out = f.on_final(final_hyp("you", 1500, 1700), 1000, &dropped);
	REQUIRE(out.size() == 1);
	CHECK(out[0].text == "you");
}

TEST_CASE("held candidate is released by following speech")
{
	FilterConfig cfg;
	HallucinationFilter f(cfg);
	bool dropped = false;
	CHECK(f.on_final(final_hyp("thank you", 10000, 10500), no_speech, &dropped).empty());
	auto out = f.on_final(final_hyp("for the raid", 11000, 12000), no_speech, &dropped);
	REQUIRE(out.size() == 2);
	CHECK(out[0].text == "thank you");
	CHECK(out[1].text == "for the raid");
	CHECK_FALSE(dropped);
}

TEST_CASE("held candidate is released by a speech partial")
{
	FilterConfig cfg;
	HallucinationFilter f(cfg);
	bool dropped = false;
	CHECK(f.on_final(final_hyp("thank you", 10000, 10500), no_speech, &dropped).empty());
	Hypothesis p = final_hyp("for the", 11000, 11400);
	p.kind = HypothesisKind::partial;
	auto released = f.on_partial(p);
	REQUIRE(released.has_value());
	CHECK(released->text == "thank you");
}

TEST_CASE("held candidate is dropped when the next speech is too late")
{
	FilterConfig cfg;
	HallucinationFilter f(cfg);
	bool dropped = false;
	CHECK(f.on_final(final_hyp("thank you", 10000, 10500), no_speech, &dropped).empty());
	auto out = f.on_final(final_hyp("hello again", 20000, 21000), no_speech, &dropped);
	REQUIRE(out.size() == 1);
	CHECK(out[0].text == "hello again");
	CHECK(dropped);
}
