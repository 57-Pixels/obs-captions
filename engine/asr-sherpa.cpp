/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "asr-sherpa.h"

#include <cstring>
#include <filesystem>

/* SherpaOnnx* online recognizer API: engine/third_party/sherpa-onnx/c-api.h
 * (sherpa-onnx v1.13.8) */
#include <c-api.h>

#include "text-case.h"

namespace fs = std::filesystem;

namespace slc::engine {

namespace {

constexpr int sample_rate = 16000;

/* First file in dir whose name starts with prefix and ends with suffix,
 * preferring int8 (smaller, faster on CPU) when prefer_int8 is set. */
std::string find_model_file(const fs::path &dir, const std::string &prefix, bool prefer_int8)
{
	std::string plain, int8;
	std::error_code ec;
	for (const auto &entry : fs::directory_iterator(dir, ec)) {
		std::string name = entry.path().filename().string();
		if (name.rfind(prefix, 0) != 0 || entry.path().extension() != ".onnx")
			continue;
		if (name.find(".int8.") != std::string::npos)
			int8 = entry.path().string();
		else
			plain = entry.path().string();
	}
	if (prefer_int8 && !int8.empty())
		return int8;
	return plain.empty() ? int8 : plain;
}

} // namespace

SherpaAsr::SherpaAsr(const std::string &model_dir, int num_threads)
{
	fs::path dir = fs::u8path(model_dir);
	model = dir.filename().string();

	std::string encoder = find_model_file(dir, "encoder", true);
	std::string decoder = find_model_file(dir, "decoder", false);
	std::string joiner = find_model_file(dir, "joiner", true);
	std::string tokens = (dir / "tokens.txt").string();
	if (encoder.empty() || decoder.empty() || joiner.empty() || !fs::exists(tokens)) {
		error_text = "model files not found in " + model_dir;
		return;
	}
	paths = {encoder, decoder, joiner, tokens};

	SherpaOnnxOnlineRecognizerConfig config;
	std::memset(&config, 0, sizeof(config));
	config.feat_config.sample_rate = sample_rate;
	config.feat_config.feature_dim = 80;
	config.model_config.transducer.encoder = paths[0].c_str();
	config.model_config.transducer.decoder = paths[1].c_str();
	config.model_config.transducer.joiner = paths[2].c_str();
	config.model_config.tokens = paths[3].c_str();
	config.model_config.num_threads = num_threads;
	config.model_config.provider = "cpu";
	config.decoding_method = "greedy_search";
	/* Endpoints: a final after 2.4 s of silence with no speech, 0.8 s
	 * after speech, or after 15 s of continuous speech. */
	config.enable_endpoint = 1;
	config.rule1_min_trailing_silence = 2.4f;
	config.rule2_min_trailing_silence = 0.8f;
	config.rule3_min_utterance_length = 15.0f;

	recognizer = SherpaOnnxCreateOnlineRecognizer(&config);
	if (!recognizer) {
		error_text = "sherpa-onnx could not load the model in " + model_dir;
		return;
	}
	stream = SherpaOnnxCreateOnlineStream(recognizer);
}

SherpaAsr::~SherpaAsr()
{
	if (stream)
		SherpaOnnxDestroyOnlineStream(stream);
	if (recognizer)
		SherpaOnnxDestroyOnlineRecognizer(recognizer);
}

void SherpaAsr::feed(const float *samples, size_t count, const std::function<void(const AsrResult &)> &on_result)
{
	if (!ok() || count == 0)
		return;
	SherpaOnnxOnlineStreamAcceptWaveform(stream, sample_rate, samples, static_cast<int32_t>(count));
	samples_total += count;
	decode(on_result, false);
}

void SherpaAsr::finish(const std::function<void(const AsrResult &)> &on_result)
{
	if (!ok())
		return;
	/* The model needs some trailing audio to emit the last word; live
	 * audio always has it, a finished file does not (as in the
	 * sherpa-onnx examples, pad with 0.66 s of silence). */
	std::vector<float> tail(sample_rate * 66 / 100, 0.0f);
	SherpaOnnxOnlineStreamAcceptWaveform(stream, sample_rate, tail.data(), static_cast<int32_t>(tail.size()));
	samples_total += tail.size();
	SherpaOnnxOnlineStreamInputFinished(stream);
	decode(on_result, true);
}

void SherpaAsr::decode(const std::function<void(const AsrResult &)> &on_result, bool force_final)
{
	while (SherpaOnnxIsOnlineStreamReady(recognizer, stream))
		SherpaOnnxDecodeOnlineStream(recognizer, stream);

	const SherpaOnnxOnlineRecognizerResult *r = SherpaOnnxGetOnlineStreamResult(recognizer, stream);
	std::string text = sentence_case(r && r->text ? r->text : "");
	SherpaOnnxDestroyOnlineRecognizerResult(r);
	/* Trim leading/trailing spaces some tokenizers produce. */
	while (!text.empty() && text.front() == ' ')
		text.erase(text.begin());
	while (!text.empty() && text.back() == ' ')
		text.pop_back();

	AsrResult result;
	result.text = text;
	result.t_start_s = static_cast<double>(utterance_start) / sample_rate;
	result.t_end_s = static_cast<double>(samples_total) / sample_rate;

	const bool endpoint = force_final || SherpaOnnxOnlineStreamIsEndpoint(recognizer, stream);
	if (endpoint) {
		if (!text.empty()) {
			result.final = true;
			result.stable_prefix_len = text.size();
			on_result(result);
		}
		SherpaOnnxOnlineStreamReset(recognizer, stream);
		stability.reset();
		last_text.clear();
		utterance_start = samples_total;
		return;
	}

	if (text != last_text && !text.empty()) {
		result.stable_prefix_len = stability.update(text);
		on_result(result);
		last_text = text;
	}
}

} // namespace slc::engine
