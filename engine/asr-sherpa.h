/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Streaming speech recognition with sherpa-onnx (CPU) and a streaming
 * zipformer transducer model directory, e.g.
 * sherpa-onnx-streaming-zipformer-en-2023-06-21. Feed 16 kHz mono audio;
 * get partial and final results. Endpoint detection (a pause after speech)
 * turns the current partial into a final.
 */

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "stability.h"

struct SherpaOnnxOnlineRecognizer;
struct SherpaOnnxOnlineStream;

namespace slc::engine {

struct AsrResult {
	bool final = false;
	std::string text;       /* sentence case */
	double t_start_s = 0.0; /* audio time since the utterance began */
	double t_end_s = 0.0;
	size_t stable_prefix_len = 0;
};

class SherpaAsr {
public:
	/* Finds encoder/decoder/joiner/tokens files in model_dir. */
	SherpaAsr(const std::string &model_dir, int num_threads);
	~SherpaAsr();

	SherpaAsr(const SherpaAsr &) = delete;
	SherpaAsr &operator=(const SherpaAsr &) = delete;

	/* False if the model could not be loaded; error() says why. */
	bool ok() const { return recognizer && stream; }
	const std::string &error() const { return error_text; }
	const std::string &model_name() const { return model; }

	/* Called once per changed partial and once per final. */
	void feed(const float *samples, size_t count, const std::function<void(const AsrResult &)> &on_result);
	/* Flush at the end of input (used by the test-wav mode). */
	void finish(const std::function<void(const AsrResult &)> &on_result);

private:
	void decode(const std::function<void(const AsrResult &)> &on_result, bool force_final);

	const SherpaOnnxOnlineRecognizer *recognizer = nullptr;
	const SherpaOnnxOnlineStream *stream = nullptr;
	std::string model;
	std::string error_text;
	std::string last_text;
	size_t samples_total = 0;
	size_t utterance_start = 0;
	std::vector<std::string> paths; /* keeps config strings alive */
	StabilityTracker stability;
};

} // namespace slc::engine
