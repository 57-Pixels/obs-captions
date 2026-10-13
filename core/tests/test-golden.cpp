/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * Golden-file tests: every core/tests/golden/*.case file is an input event
 * script plus the exact caption updates it must produce. Format:
 *
 *   # comment
 *   config cols=28 rows=2 show_stable_partial=1 ...
 *   ticks <step_ms> <until_ms>          tick(now) at every multiple of step
 *   at <ms> final "text" start=<ms> end=<ms> [speaker=<id>]
 *   at <ms> partial "text" start=<ms> end=<ms> stable=<bytes> [speaker=<id>]
 *   at <ms> tts "text" duration=<ms> [speaker=<id>]
 *   ---
 *   <ms> "expected caption text"         one line per CaptionUpdate
 *   stat <name> <value>                  checked after the last tick
 *
 * Strings use C escapes: \n for a line break, \" and \\. Events at a given
 * time are pushed before the tick at that time. The runner prints the actual
 * output in the same format when a case fails.
 */

#include <doctest/doctest.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "obscap/pipeline.h"

namespace fs = std::filesystem;
using namespace obscap;

namespace {

/* Repeated keys are allowed (several replace=from:to rules). */
using Args = std::multimap<std::string, std::string>;

struct Event {
	int64_t at = 0;
	enum { final, partial, tts } type = final;
	std::string text;
	Args args;
};

struct Case {
	Config config;
	int64_t tick_step = 100;
	int64_t tick_until = 0;
	std::vector<Event> events;
	std::vector<std::string> expected;
	std::map<std::string, int64_t> expected_stats;
};

std::string escape(const std::string &s)
{
	std::string out;
	for (char c : s) {
		if (c == '\n')
			out += "\\n";
		else if (c == '"')
			out += "\\\"";
		else if (c == '\\')
			out += "\\\\";
		else
			out += c;
	}
	return out;
}

/* Parses a quoted string starting at in[pos] == '"'; advances pos. */
std::string parse_quoted(const std::string &in, size_t &pos)
{
	std::string out;
	REQUIRE(in[pos] == '"');
	for (pos++; pos < in.size() && in[pos] != '"'; pos++) {
		if (in[pos] == '\\' && pos + 1 < in.size()) {
			char n = in[++pos];
			out += n == 'n' ? '\n' : n;
		} else {
			out += in[pos];
		}
	}
	REQUIRE(pos < in.size());
	pos++;
	return out;
}

Args parse_args(std::istringstream &in)
{
	Args args;
	std::string kv;
	while (in >> kv) {
		size_t eq = kv.find('=');
		REQUIRE_MESSAGE(eq != std::string::npos, "bad argument: " << kv);
		args.emplace(kv.substr(0, eq), kv.substr(eq + 1));
	}
	return args;
}

void apply_config(Config &c, const Args &args)
{
	for (const auto &[key, value] : args) {
		if (key == "cols")
			c.formatter.cols = std::stoi(value);
		else if (key == "rows")
			c.formatter.rows = std::stoi(value);
		else if (key == "min_interval_ms")
			c.scheduler.min_interval_ms = std::stoll(value);
		else if (key == "clear_after_ms")
			c.scheduler.clear_after_ms = std::stoll(value);
		else if (key == "show_stable_partial")
			c.scheduler.show_stable_partial = value == "1";
		else if (key == "repeat_threshold")
			c.filters.repeat_threshold = std::stoi(value);
		else if (key == "hallucination_filter")
			c.filters.hallucination_filter = value == "1";
		else if (key == "profanity_mask")
			c.filters.profanity_mask = value == "1";
		else if (key == "replace") {
			size_t colon = value.find(':');
			REQUIRE(colon != std::string::npos);
			std::string from = value.substr(0, colon), to = value.substr(colon + 1);
			std::replace(from.begin(), from.end(), '_', ' ');
			std::replace(to.begin(), to.end(), '_', ' ');
			c.filters.word_replacements.emplace_back(from, to);
		} else
			FAIL("unknown config key: " << key);
	}
}

Case load(const fs::path &path)
{
	std::ifstream file(path);
	REQUIRE_MESSAGE(file, "cannot open " << path.string());
	Case c;
	bool in_expected = false;
	std::string line;
	while (std::getline(file, line)) {
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		size_t first = line.find_first_not_of(" \t");
		if (first == std::string::npos || line[first] == '#')
			continue;
		if (line == "---") {
			in_expected = true;
			continue;
		}
		std::istringstream in(line);
		std::string word;
		in >> word;
		if (in_expected) {
			if (word == "stat") {
				std::string name;
				int64_t value = 0;
				in >> name >> value;
				c.expected_stats[name] = value;
			} else {
				size_t q = line.find('"');
				REQUIRE_MESSAGE(q != std::string::npos, "bad expected line: " << line);
				std::string text = parse_quoted(line, q);
				c.expected.push_back(word + " \"" + escape(text) + "\"");
			}
		} else if (word == "config") {
			apply_config(c.config, parse_args(in));
		} else if (word == "ticks") {
			in >> c.tick_step >> c.tick_until;
		} else if (word == "at") {
			Event e;
			std::string type;
			in >> e.at >> type;
			e.type = type == "final" ? Event::final : type == "partial" ? Event::partial : Event::tts;
			REQUIRE_MESSAGE((type == "final" || type == "partial" || type == "tts"), "bad type " << type);
			size_t q = line.find('"');
			REQUIRE(q != std::string::npos);
			e.text = parse_quoted(line, q);
			std::istringstream rest(line.substr(q));
			e.args = parse_args(rest);
			c.events.push_back(e);
		} else {
			FAIL("unknown directive: " << line);
		}
	}
	return c;
}

int64_t arg(const Event &e, const char *name, int64_t fallback = 0)
{
	auto it = e.args.find(name);
	return it == e.args.end() ? fallback : std::stoll(it->second);
}

void push(CaptionPipeline &p, const Event &e)
{
	auto it = e.args.find("speaker");
	std::string speaker = it == e.args.end() ? "" : it->second;
	if (e.type == Event::tts) {
		p.push(TextEvent{e.text, e.at, arg(e, "duration"), speaker});
		return;
	}
	Hypothesis h;
	h.kind = e.type == Event::final ? HypothesisKind::final : HypothesisKind::partial;
	h.text = e.text;
	h.t_start_ms = arg(e, "start");
	h.t_end_ms = arg(e, "end", e.at);
	h.stable_prefix_len = static_cast<size_t>(arg(e, "stable"));
	h.speaker = speaker;
	p.push(h);
}

int64_t stat_value(const Stats &s, const std::string &name)
{
	if (name == "emitted")
		return static_cast<int64_t>(s.emitted);
	if (name == "stripped_chars")
		return static_cast<int64_t>(s.stripped_chars);
	if (name == "lag_ms")
		return s.lag_ms;
	for (size_t i = 0; i < static_cast<size_t>(SuppressReason::count); i++) {
		auto reason = static_cast<SuppressReason>(i);
		if (name == std::string("suppressed.") + suppress_reason_name(reason))
			return static_cast<int64_t>(s.suppressed_by(reason));
	}
	FAIL("unknown stat: " << name);
	return 0;
}

std::string join(const std::vector<std::string> &lines)
{
	std::string out;
	for (const auto &l : lines)
		out += l + "\n";
	return out;
}

void run(const fs::path &path)
{
	Case c = load(path);
	CaptionPipeline pipeline(c.config);
	std::vector<std::string> actual;
	size_t next = 0;
	for (int64_t now = 0; now <= c.tick_until; now += c.tick_step) {
		while (next < c.events.size() && c.events[next].at <= now)
			push(pipeline, c.events[next++]);
		if (auto update = pipeline.tick(now)) {
			CHECK_MESSAGE(update->display_duration_ms == pipeline.config().scheduler.min_interval_ms,
				      "display_duration at " << now);
			actual.push_back(std::to_string(now) + " \"" + escape(update->text) + "\"");
		}
	}
	CHECK_MESSAGE(next == c.events.size(), "events after the last tick");
	CHECK_MESSAGE(join(actual) == join(c.expected), "case " << path.filename().string() << "\n--- actual\n"
								<< join(actual) << "--- expected\n"
								<< join(c.expected));
	for (const auto &[name, value] : c.expected_stats)
		CHECK_MESSAGE(stat_value(pipeline.stats(), name) == value,
			      path.filename().string()
				      << ": stat " << name << " = " << stat_value(pipeline.stats(), name));
}

} // namespace

TEST_CASE("golden caption cases")
{
	std::vector<fs::path> cases;
	for (const auto &entry : fs::directory_iterator(OBSCAP_GOLDEN_DIR))
		if (entry.path().extension() == ".case")
			cases.push_back(entry.path());
	std::sort(cases.begin(), cases.end());
	REQUIRE(!cases.empty());
	for (const auto &path : cases) {
		CAPTURE(path.filename().string());
		run(path);
	}
}
