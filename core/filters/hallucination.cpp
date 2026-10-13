/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "filters/hallucination.h"

#include <algorithm>

#include "filters/text-norm.h"

namespace slc {

HallucinationFilter::HallucinationFilter(const FilterConfig &config)
	: enabled(config.hallucination_filter),
	  max_words(config.hallucination_max_words),
	  isolation_ms(config.hallucination_isolation_ms)
{
	for (const auto &entry : config.hallucination_blocklist)
		blocklist.push_back(normalize(entry));
}

bool HallucinationFilter::is_candidate(std::string_view text) const
{
	if (!enabled)
		return false;
	std::string norm = normalize(text);
	if (norm.empty() || word_count(norm) > static_cast<size_t>(max_words))
		return false;
	return std::find(blocklist.begin(), blocklist.end(), norm) != blocklist.end();
}

bool HallucinationFilter::releases(int64_t speech_start_ms) const
{
	return held && speech_start_ms - held->t_end_ms <= isolation_ms;
}

std::vector<Hypothesis> HallucinationFilter::on_final(const Hypothesis &final_hyp, int64_t last_speech_end_ms,
						      bool *dropped)
{
	std::vector<Hypothesis> accepted;
	*dropped = false;

	const bool candidate = is_candidate(final_hyp.text);
	if (held) {
		/* Only real speech releases a held candidate. Another blocklisted
		 * phrase does not: silence artifacts often come in a row
		 * ("Thank you." "Thank you."), and must not vouch for each other. */
		if (!candidate && releases(final_hyp.t_start_ms)) {
			accepted.push_back(*held);
			last_speech_end_ms = held->t_end_ms;
		} else {
			*dropped = true;
		}
		held.reset();
	}

	if (candidate && final_hyp.t_start_ms - last_speech_end_ms > isolation_ms)
		held = final_hyp;
	else
		accepted.push_back(final_hyp);
	return accepted;
}

std::optional<Hypothesis> HallucinationFilter::on_partial(const Hypothesis &partial)
{
	std::string norm = normalize(partial.text);
	if (norm.empty() || is_candidate(partial.text) || !releases(partial.t_start_ms))
		return std::nullopt;
	std::optional<Hypothesis> released = held;
	held.reset();
	return released;
}

bool HallucinationFilter::on_tick(int64_t now_ms)
{
	if (!held || now_ms < held->t_end_ms + isolation_ms)
		return false;
	held.reset();
	return true;
}

void HallucinationFilter::reset()
{
	held.reset();
}

} // namespace slc
