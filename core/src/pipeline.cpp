/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "obscap/pipeline.h"

#include <algorithm>
#include <limits>

#include "filters/hallucination.h"
#include "filters/profanity.h"
#include "filters/repeat.h"
#include "filters/text-norm.h"
#include "filters/word-replace.h"
#include "formatter/transliterate.h"
#include "scheduler/scheduler.h"

namespace obscap {

namespace {

constexpr int64_t no_speech = std::numeric_limits<int64_t>::min() / 4;

} // namespace

/*
 * Order of operations for a final:
 *   TTS check -> word replacement -> hallucination (may hold) ->
 *   in-utterance repeat collapse -> repeat filter -> profanity mask ->
 *   transliteration -> scheduler.
 */
struct CaptionPipeline::Impl {
	explicit Impl(const Config &config)
		: cfg(sanitize(config)),
		  repeat(cfg.filters),
		  hallucination(cfg.filters),
		  scheduler(cfg.formatter, cfg.scheduler)
	{
	}

	void count(SuppressReason reason) { stats.suppressed[static_cast<size_t>(reason)]++; }

	bool overlaps_tts(const Hypothesis &h) const
	{
		return tts_end_ms > tts_start_ms && h.t_start_ms < tts_end_ms && h.t_end_ms > tts_start_ms;
	}

	std::string finish(std::string text, bool count_stripped)
	{
		if (cfg.filters.profanity_mask)
			text = mask_profanity(text, cfg.filters.profanity_words);
		Transliterated t = transliterate_608(text);
		if (count_stripped)
			stats.stripped_chars += t.stripped;
		return t.text;
	}

	void accept(const Hypothesis &h)
	{
		std::string text = collapse_repeats(h.text, cfg.filters.repeat_threshold);
		if (normalize(text).empty())
			return;
		last_speech_end_ms = std::max(last_speech_end_ms, h.t_end_ms);
		if (repeat.suppress(text, h.t_start_ms, h.t_end_ms)) {
			count(SuppressReason::repeat);
			return;
		}
		scheduler.commit(finish(text, true), h.speaker, h.t_end_ms);
	}

	void push_partial(const Hypothesis &h)
	{
		if (auto released = hallucination.on_partial(h))
			accept(*released);

		std::string stable = stable_words(h.text, h.stable_prefix_len);
		if (hallucination.is_candidate(stable))
			stable.clear(); /* might still turn out to be an artifact */
		stable = replace_words(stable, cfg.filters.word_replacements);
		scheduler.set_partial(finish(stable, false), h.speaker, h.t_end_ms);
	}

	void push_final(const Hypothesis &h)
	{
		scheduler.set_partial("", h.speaker, h.t_end_ms);
		Hypothesis replaced = h;
		replaced.text = replace_words(h.text, cfg.filters.word_replacements);
		bool dropped = false;
		for (const auto &accepted : hallucination.on_final(replaced, last_speech_end_ms, &dropped))
			accept(accepted);
		if (dropped)
			count(SuppressReason::hallucination);
	}

	Config cfg;
	Stats stats;
	RepeatFilter repeat;
	HallucinationFilter hallucination;
	Scheduler scheduler;
	int64_t last_speech_end_ms = no_speech;
	int64_t tts_start_ms = 0;
	int64_t tts_end_ms = 0;
};

CaptionPipeline::CaptionPipeline(const Config &config) : impl(std::make_unique<Impl>(config)) {}

CaptionPipeline::~CaptionPipeline() = default;

void CaptionPipeline::push(const Hypothesis &hypothesis)
{
	if (impl->overlaps_tts(hypothesis)) {
		/* Most likely ASR hearing the TTS voice itself. */
		if (hypothesis.kind == HypothesisKind::final)
			impl->count(SuppressReason::tts_active);
		return;
	}
	if (hypothesis.kind == HypothesisKind::partial)
		impl->push_partial(hypothesis);
	else
		impl->push_final(hypothesis);
}

void CaptionPipeline::push(const TextEvent &text)
{
	/* TTS lines are exact: no replacement, repeat or hallucination
	 * filtering, only the profanity mask and transliteration. */
	const int64_t end = text.start_ms + std::max<int64_t>(text.est_duration_ms, 0);
	impl->tts_start_ms = text.start_ms;
	impl->tts_end_ms = end;
	impl->last_speech_end_ms = std::max(impl->last_speech_end_ms, end);
	impl->scheduler.set_partial("", text.speaker, text.start_ms);
	impl->scheduler.commit(impl->finish(text.text, true), text.speaker, end);
}

std::optional<CaptionUpdate> CaptionPipeline::tick(int64_t now_ms)
{
	if (impl->hallucination.on_tick(now_ms))
		impl->count(SuppressReason::hallucination);
	return impl->scheduler.tick(now_ms, impl->stats);
}

void CaptionPipeline::reset()
{
	impl->repeat.reset();
	impl->hallucination.reset();
	impl->scheduler.reset();
	impl->last_speech_end_ms = no_speech;
	impl->tts_start_ms = 0;
	impl->tts_end_ms = 0;
}

const Stats &CaptionPipeline::stats() const
{
	return impl->stats;
}

const Config &CaptionPipeline::config() const
{
	return impl->cfg;
}

} // namespace obscap
