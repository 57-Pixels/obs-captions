/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "scheduler/scheduler.h"

#include <algorithm>

#include "formatter/wrap.h"

namespace slc {

namespace {

std::string trim_right(std::string s)
{
	while (!s.empty() && s.back() == ' ')
		s.pop_back();
	return s;
}

} // namespace

std::string stable_words(std::string_view text, size_t stable_prefix_len)
{
	if (stable_prefix_len >= text.size())
		return trim_right(std::string(text));
	std::string prefix(text.substr(0, stable_prefix_len));
	if (text[stable_prefix_len] != ' ') {
		/* The stable part ends inside a word: drop that partial word. */
		size_t space = prefix.rfind(' ');
		prefix = space == std::string::npos ? std::string() : prefix.substr(0, space);
	}
	return trim_right(prefix);
}

Scheduler::Scheduler(const FormatterConfig &formatter, const SchedulerConfig &scheduler)
	: fmt(formatter),
	  cfg(scheduler)
{
}

bool Scheduler::is_new_speaker(const std::string &speaker) const
{
	return !speaker.empty() && !last_speaker.empty() && speaker != last_speaker;
}

void Scheduler::commit(const std::string &text, const std::string &speaker, int64_t t_end_ms)
{
	if (text.empty())
		return;
	segments.push_back({text, is_new_speaker(speaker), t_end_ms});
	if (!speaker.empty())
		last_speaker = speaker;
	if (segments.size() > max_segments)
		segments.erase(segments.begin(), segments.begin() + trim_batch);
}

void Scheduler::set_partial(const std::string &text, const std::string &speaker, int64_t t_end_ms)
{
	if (text.empty())
		partial.reset();
	else
		partial = Segment{text, is_new_speaker(speaker), t_end_ms};
}

int64_t Scheduler::newest_end_ms() const
{
	int64_t newest = partial ? partial->t_end_ms : 0;
	for (const auto &s : segments)
		newest = std::max(newest, s.t_end_ms);
	return newest;
}

std::string Scheduler::render() const
{
	std::vector<std::string> paragraphs;
	auto add = [&](const Segment &s) {
		if (s.new_speaker)
			paragraphs.push_back(">> " + s.text);
		else if (paragraphs.empty())
			paragraphs.push_back(s.text);
		else
			paragraphs.back() += " " + s.text;
	};
	for (const auto &s : segments)
		add(s);
	if (cfg.show_stable_partial && partial)
		add(*partial);
	return layout(paragraphs, fmt.cols, fmt.rows);
}

std::optional<CaptionUpdate> Scheduler::tick(int64_t now_ms, Stats &stats)
{
	if ((!segments.empty() || partial) && now_ms >= newest_end_ms() + cfg.clear_after_ms) {
		segments.clear();
		partial.reset();
	}

	const std::string display = render();
	std::optional<CaptionUpdate> update;

	if (display == last_sent) {
		waiting.reset();
	} else if (last_sent_ms && now_ms - *last_sent_ms < cfg.min_interval_ms) {
		if (waiting && *waiting != display)
			stats.suppressed[static_cast<size_t>(SuppressReason::superseded)]++;
		waiting = display;
	} else {
		last_sent = display;
		last_sent_ms = now_ms;
		waiting.reset();
		sent_newest_end_ms = newest_end_ms();
		stats.emitted++;
		update = CaptionUpdate{display, cfg.min_interval_ms};
	}

	stats.lag_ms = last_sent.empty() ? 0 : std::max<int64_t>(0, now_ms - sent_newest_end_ms);
	return update;
}

void Scheduler::reset()
{
	segments.clear();
	partial.reset();
	last_speaker.clear();
	last_sent.clear();
	waiting.reset();
	last_sent_ms.reset();
	sent_newest_end_ms = 0;
}

} // namespace slc
