/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "filters/repeat.h"

#include <vector>

#include "filters/text-norm.h"

namespace slc {

namespace {

constexpr size_t max_phrase_words = 6;

std::vector<std::string> split_spaces(std::string_view text)
{
	std::vector<std::string> out;
	size_t i = 0;
	while (i < text.size()) {
		size_t end = text.find(' ', i);
		if (end == std::string_view::npos)
			end = text.size();
		if (end > i)
			out.emplace_back(text.substr(i, end - i));
		i = end + 1;
	}
	return out;
}

/* Number of times words[at, at+n) repeats back to back from at. */
size_t run_length(const std::vector<std::string> &norm, size_t at, size_t n)
{
	size_t count = 1;
	while (at + (count + 1) * n <= norm.size()) {
		bool same = true;
		for (size_t k = 0; k < n && same; k++)
			same = norm[at + k] == norm[at + count * n + k];
		if (!same)
			break;
		count++;
	}
	return count;
}

} // namespace

std::string collapse_repeats(std::string_view text, int threshold)
{
	const std::vector<std::string> words = split_spaces(text);
	std::vector<std::string> norm;
	for (const auto &w : words)
		norm.push_back(normalize(w));

	const size_t limit = threshold > 1 ? static_cast<size_t>(threshold) : 2;
	std::vector<std::string> out;
	size_t i = 0;
	while (i < words.size()) {
		bool collapsed = false;
		for (size_t n = 1; n <= max_phrase_words && i + n * limit <= words.size(); n++) {
			if (norm[i].empty())
				break;
			size_t count = run_length(norm, i, n);
			if (count < limit)
				continue;
			for (size_t k = 0; k < (limit - 1) * n; k++)
				out.push_back(words[i + k]);
			i += count * n;
			collapsed = true;
			break;
		}
		if (!collapsed)
			out.push_back(words[i++]);
	}

	std::string result;
	for (const auto &w : out) {
		if (!result.empty())
			result += ' ';
		result += w;
	}
	return result;
}

RepeatFilter::RepeatFilter(const FilterConfig &config)
	: threshold(config.repeat_threshold),
	  window_ms(config.repeat_window_ms),
	  ratio(config.near_duplicate_ratio)
{
}

bool RepeatFilter::suppress(std::string_view text, int64_t t_start_ms, int64_t t_end_ms)
{
	std::string norm = normalize(text);
	bool continues = run > 0 && t_start_ms - last_end_ms <= window_ms && edit_ratio(norm, last_norm) <= ratio;
	run = continues ? run + 1 : 1;
	last_norm = std::move(norm);
	last_end_ms = t_end_ms;
	return run >= threshold;
}

void RepeatFilter::reset()
{
	last_norm.clear();
	last_end_ms = 0;
	run = 0;
}

} // namespace slc
