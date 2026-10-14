/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * simple-local-captions-engine: runs speech recognition for the OBS plugin
 * in its own process (ADR 0001), at below-normal priority.
 *
 *   simple-local-captions-engine --pipe <name> --model <dir>
 *       Normal mode, started by the plugin (protocol/PROTOCOL.md).
 *   simple-local-captions-engine --model <dir> --test-wav <file.wav>
 *       Transcribes a 16 kHz mono 16-bit WAV and prints the results; for
 *       checking a model without OBS.
 */

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "asr-sherpa.h"
#include "engine-api.h"
#include "pipe-client.h"
#include "protocol.h"

using namespace slc::engine;

namespace {

constexpr const char *engine_version = "0.1.0";
constexpr const char *backend_name = "sherpa-onnx-zipformer";
/* One thread is as fast as two for this model (real-time factor ~0.12 on
 * the test files) and avoids ONNX Runtime worker threads spinning. */
constexpr int asr_threads = 1;

struct Args {
	std::string pipe;
	std::string model;
	std::string test_wav;
};

std::string to_utf8(const wchar_t *w)
{
	int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
	std::string out(n > 0 ? static_cast<size_t>(n - 1) : 0, '\0');
	if (n > 1)
		WideCharToMultiByte(CP_UTF8, 0, w, -1, out.data(), n, nullptr, nullptr);
	return out;
}

/* wmain + UTF-8 so model paths under non-ASCII user names work. */
Args parse_args(int argc, wchar_t **argv)
{
	Args a;
	for (int i = 1; i + 1 < argc; i += 2) {
		std::string key = to_utf8(argv[i]);
		std::string value = to_utf8(argv[i + 1]);
		if (key == "--pipe")
			a.pipe = value;
		else if (key == "--model")
			a.model = value;
		else if (key == "--test-wav")
			a.test_wav = value;
	}
	return a;
}

/* Minimal reader for 16 kHz mono PCM16 WAV files (the model's test files). */
bool read_wav(const std::string &path, std::vector<float> &out)
{
	std::ifstream f(std::filesystem::u8path(path), std::ios::binary);
	std::vector<char> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	if (data.size() < 44 || std::memcmp(data.data(), "RIFF", 4) != 0)
		return false;
	size_t pos = 12;
	while (pos + 8 <= data.size()) {
		uint32_t len;
		std::memcpy(&len, data.data() + pos + 4, 4);
		if (std::memcmp(data.data() + pos, "data", 4) == 0) {
			size_t n = std::min<size_t>(len, data.size() - pos - 8) / 2;
			out.resize(n);
			for (size_t i = 0; i < n; i++) {
				int16_t v;
				std::memcpy(&v, data.data() + pos + 8 + 2 * i, 2);
				out[i] = static_cast<float>(v) / 32768.0f;
			}
			return true;
		}
		pos += 8 + len + (len & 1);
	}
	return false;
}

int run_test_wav(const Args &args)
{
	SherpaAsr asr(args.model, asr_threads);
	if (!asr.ok()) {
		std::fprintf(stderr, "error: %s\n", asr.error().c_str());
		return 1;
	}
	std::vector<float> samples;
	if (!read_wav(args.test_wav, samples)) {
		std::fprintf(stderr, "error: cannot read %s (16 kHz mono PCM16 WAV)\n", args.test_wav.c_str());
		return 1;
	}
	auto print = [](const AsrResult &r) {
		if (r.final)
			std::printf("final   [%.2f-%.2f] %s\n", r.t_start_s, r.t_end_s, r.text.c_str());
	};
	ULONGLONG start = GetTickCount64();
	const size_t chunk = 1600; /* 100 ms, like live audio */
	for (size_t i = 0; i < samples.size(); i += chunk)
		asr.feed(samples.data() + i, std::min(chunk, samples.size() - i), print);
	asr.finish(print);
	double audio_s = static_cast<double>(samples.size()) / 16000.0;
	double cpu_s = static_cast<double>(GetTickCount64() - start) / 1000.0;
	std::printf("audio %.1f s, processed in %.1f s (real-time factor %.2f)\n", audio_s, cpu_s, cpu_s / audio_s);
	return 0;
}

int run_pipe(const Args &args)
{
	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

	PipeClient pipe;
	if (!pipe.connect(args.pipe, 10000))
		return 2;
	pipe.write_json(hello_message(engine_version, backend_name));

	SherpaAsr asr(args.model, asr_threads);
	if (!asr.ok()) {
		pipe.write_json(error_message("model_missing", asr.error(), "Engine.Error.ModelMissing"));
		return 3;
	}
	pipe.write_json(status_message(asr.model_name().c_str(), "listening"));

	auto send = [&pipe](const AsrResult &r) {
		pipe.write_json(hypothesis_message(r.final, r.text, r.t_start_s, r.t_end_s, r.stable_prefix_len));
	};

	FrameReader reader;
	uint8_t buf[16384];
	for (;;) {
		size_t got = pipe.read(buf, sizeof(buf));
		if (got == 0)
			return 0; /* plugin closed the pipe */
		reader.feed(buf, got);
		while (auto msg = reader.next()) {
			if (msg->type == frame_audio)
				asr.feed(msg->audio.samples.data(), msg->audio.samples.size(), send);
			else if (json_type(msg->json) == "shutdown")
				return 0;
		}
		if (reader.broken()) {
			pipe.write_json(error_message("protocol", "corrupt frame from plugin", "Engine.Error.Crashed"));
			return 4;
		}
	}
}

} // namespace

int wmain(int argc, wchar_t **argv)
{
	Args args = parse_args(argc, argv);
	if (args.model.empty() || (args.pipe.empty() && args.test_wav.empty())) {
		std::fprintf(stderr,
			     "simple-local-captions-engine (protocol v%u)\n"
			     "usage: --pipe <name> --model <dir> | --model <dir> --test-wav <file.wav>\n",
			     slc::engine::protocol_version);
		return 1;
	}
	return args.test_wav.empty() ? run_pipe(args) : run_test_wav(args);
}
