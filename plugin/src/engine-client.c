/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "engine-client.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

/* obs_current_module: libobs/obs-module.h; obs_get_module_binary_path,
 * obs_data_create_from_json, obs_data_get_string, obs_data_get_double,
 * obs_data_get_int, obs_data_release: libobs/obs.h, libobs/obs-data.h */
#include <obs-module.h>
/* audio_resampler_create, audio_resampler_resample,
 * audio_resampler_destroy, struct resample_info:
 * libobs/media-io/audio-resampler.h */
#include <media-io/audio-resampler.h>
/* os_gettime_ns: libobs/util/platform.h */
#include <util/platform.h>
/* pthread_create, pthread_join, os_set_thread_name, os_atomic_*:
 * libobs/util/threading.h */
#include <util/threading.h>

#include <plugin-support.h>
#include <slc/c-api.h>

#include "audio-tap.h"
#include "caption-out.h"

#define ENGINE_EXE L"simple-local-captions-engine.exe"
#define MODEL_NAME L"sherpa-onnx-streaming-zipformer-en-2023-06-26"
#define LOOP_MS 20
#define TICK_MS 100
#define CONNECT_TIMEOUT_MS 15000
#define BACKOFF_MAX_MS 30000
/* Audio per frame: up to 100 ms at OBS's rate. */
#define FRAME_IN_SAMPLES 4800
#define LINE_MAX 65536

enum engine_state {
	ENGINE_OFF,
	ENGINE_STARTING,
	ENGINE_LISTENING,
	ENGINE_MODEL_MISSING,
	ENGINE_NOT_FOUND,
	ENGINE_RESTARTING,
};

static volatile long state = ENGINE_OFF;
static volatile long reset_requested;
static HANDLE stop_event;
static pthread_t thread;
static bool running;

struct session {
	HANDLE pipe;
	HANDLE process;
	OVERLAPPED read_ov;
	uint8_t read_buf[8192];
	char line[LINE_MAX];
	size_t line_len;
	bool in_line;
	uint32_t seq;
	bool listening;
};

static slc_pipeline *pipeline;
static audio_resampler_t *resampler;

static void set_state(enum engine_state s)
{
	os_atomic_set_long(&state, (long)s);
}

static int64_t now_ms(void)
{
	return (int64_t)(os_gettime_ns() / 1000000ULL);
}

static bool stopping(void)
{
	return WaitForSingleObject(stop_event, 0) == WAIT_OBJECT_0;
}

/* <plugin dll dir>\engine\ as a wide string. */
static bool engine_dir(wchar_t *out, size_t size)
{
	const char *dll = obs_get_module_binary_path(obs_current_module());
	if (!dll || MultiByteToWideChar(CP_UTF8, 0, dll, -1, out, (int)size) == 0)
		return false;
	wchar_t *slash = wcsrchr(out, L'\\');
	wchar_t *fwd = wcsrchr(out, L'/');
	if (fwd > slash)
		slash = fwd;
	if (!slash)
		return false;
	slash[1] = 0;
	return wcscat_s(out, size, L"engine") == 0;
}

static bool write_all(struct session *s, const void *data, size_t size)
{
	OVERLAPPED ov = {0};
	ov.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	DWORD written = 0;
	BOOL ok = WriteFile(s->pipe, data, (DWORD)size, NULL, &ov);
	if (!ok && GetLastError() == ERROR_IO_PENDING)
		ok = TRUE;
	if (ok)
		ok = GetOverlappedResult(s->pipe, &ov, &written, TRUE);
	CloseHandle(ov.hEvent);
	return ok && written == size;
}

static void write_json(struct session *s, const char *json)
{
	size_t len = strlen(json);
	char *framed = bmalloc(len + 2);
	framed[0] = 0x02;
	memcpy(framed + 1, json, len);
	framed[len + 1] = '\n';
	write_all(s, framed, len + 2);
	bfree(framed);
}

static bool launch(struct session *s)
{
	wchar_t dir[MAX_PATH], exe[MAX_PATH], models[MAX_PATH], pipe_path[128];
	wchar_t cmd[3 * MAX_PATH];
	wchar_t pipe_name[96];

	if (!engine_dir(dir, MAX_PATH) || swprintf(exe, MAX_PATH, L"%s\\%s", dir, ENGINE_EXE) < 0 ||
	    GetFileAttributesW(exe) == INVALID_FILE_ATTRIBUTES) {
		obs_log(LOG_ERROR, "speech engine not found next to the plugin");
		set_state(ENGINE_NOT_FOUND);
		return false;
	}

	wchar_t local[MAX_PATH];
	if (!GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH))
		local[0] = 0;
	swprintf(models, MAX_PATH, L"%s\\simple-local-captions\\models\\%s", local, MODEL_NAME);

	/* Random-ish name, one instance, local clients only. */
	swprintf(pipe_name, 96, L"simple-local-captions-%lu-%llu", GetCurrentProcessId(),
		 (unsigned long long)os_gettime_ns());
	swprintf(pipe_path, 128, L"\\\\.\\pipe\\%s", pipe_name);
	s->pipe = CreateNamedPipeW(pipe_path, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE,
				   PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS, 1,
				   65536, 65536, 0, NULL);
	if (s->pipe == INVALID_HANDLE_VALUE) {
		s->pipe = NULL;
		obs_log(LOG_ERROR, "could not create the engine pipe (error %lu)", GetLastError());
		return false;
	}

	swprintf(cmd, 3 * MAX_PATH, L"\"%s\" --pipe %s --model \"%s\"", exe, pipe_name, models);
	STARTUPINFOW si = {sizeof(si)};
	PROCESS_INFORMATION pi = {0};
	if (!CreateProcessW(exe, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW | BELOW_NORMAL_PRIORITY_CLASS, NULL, dir, &si,
			    &pi)) {
		obs_log(LOG_ERROR, "could not start the speech engine (error %lu)", GetLastError());
		set_state(ENGINE_NOT_FOUND);
		return false;
	}
	CloseHandle(pi.hThread);
	s->process = pi.hProcess;

	OVERLAPPED ov = {0};
	ov.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	bool connected = ConnectNamedPipe(s->pipe, &ov) != 0;
	if (!connected) {
		DWORD err = GetLastError();
		if (err == ERROR_PIPE_CONNECTED) {
			connected = true;
		} else if (err == ERROR_IO_PENDING) {
			HANDLE wait[3] = {ov.hEvent, s->process, stop_event};
			DWORD w = WaitForMultipleObjects(3, wait, FALSE, CONNECT_TIMEOUT_MS);
			DWORD dummy;
			connected = w == WAIT_OBJECT_0 && GetOverlappedResult(s->pipe, &ov, &dummy, FALSE);
			if (!connected)
				CancelIo(s->pipe);
		}
	}
	CloseHandle(ov.hEvent);
	if (!connected)
		obs_log(LOG_WARNING, "speech engine did not connect");
	return connected;
}

static void close_session(struct session *s)
{
	if (s->pipe) {
		CancelIo(s->pipe);
		CloseHandle(s->pipe); /* engine sees EOF and exits */
	}
	if (s->process) {
		if (WaitForSingleObject(s->process, 2000) != WAIT_OBJECT_0)
			TerminateProcess(s->process, 1);
		CloseHandle(s->process);
	}
	if (s->read_ov.hEvent)
		CloseHandle(s->read_ov.hEvent);
	memset(s, 0, sizeof(*s));
}

static void handle_line(struct session *s, const char *json)
{
	obs_data_t *msg = obs_data_create_from_json(json);
	if (!msg)
		return;
	const char *type = obs_data_get_string(msg, "type");

	if (strcmp(type, "hello") == 0) {
		char ack[128];
		snprintf(ack, sizeof(ack), "{\"type\":\"hello_ack\",\"protocol\":0,\"plugin_version\":\"%s\"}",
			 PLUGIN_VERSION);
		write_json(s, ack);
	} else if (strcmp(type, "status") == 0) {
		if (strcmp(obs_data_get_string(msg, "state"), "listening") == 0) {
			s->listening = true;
			audio_tap_clear(); /* drop audio queued while the model loaded */
			set_state(ENGINE_LISTENING);
			obs_log(LOG_INFO, "speech engine listening (model %s)", obs_data_get_string(msg, "model"));
		}
	} else if (strcmp(type, "error") == 0) {
		obs_log(LOG_WARNING, "speech engine error: %s", obs_data_get_string(msg, "message"));
		if (strcmp(obs_data_get_string(msg, "code"), "model_missing") == 0)
			set_state(ENGINE_MODEL_MISSING);
	} else if (strcmp(type, "hypothesis") == 0) {
		/* Map engine audio time to our clock: the result describes audio
		 * that just ended, lasting t_end - t_start. */
		int64_t duration =
			(int64_t)((obs_data_get_double(msg, "t_end") - obs_data_get_double(msg, "t_start")) * 1000.0);
		int64_t now = now_ms();
		int is_final = strcmp(obs_data_get_string(msg, "kind"), "final") == 0;
		slc_pipeline_push(pipeline, is_final, obs_data_get_string(msg, "text"), now - duration, now,
				  (size_t)obs_data_get_int(msg, "stable_prefix_len"));
	}
	obs_data_release(msg);
}

/* Engine -> plugin stream: 0x02 + JSON + '\n', repeated. */
static void handle_bytes(struct session *s, const uint8_t *data, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		uint8_t c = data[i];
		if (!s->in_line) {
			s->in_line = c == 0x02;
			s->line_len = 0;
			continue;
		}
		if (c == '\n') {
			s->line[s->line_len] = 0;
			handle_line(s, s->line);
			s->in_line = false;
		} else if (s->line_len + 1 < LINE_MAX) {
			s->line[s->line_len++] = (char)c;
		}
	}
}

static bool start_read(struct session *s)
{
	ResetEvent(s->read_ov.hEvent);
	if (ReadFile(s->pipe, s->read_buf, sizeof(s->read_buf), NULL, &s->read_ov))
		return true; /* completed now; the event is signaled */
	return GetLastError() == ERROR_IO_PENDING;
}

static void put_u32(uint8_t *p, uint32_t v)
{
	for (int i = 0; i < 4; i++)
		p[i] = (uint8_t)(v >> (8 * i));
}

/* Drain the ring buffer, resample to 16 kHz s16 mono, send audio frames. */
static bool send_audio(struct session *s)
{
	static float in[FRAME_IN_SAMPLES];
	static uint8_t frame[21 + 2 * FRAME_IN_SAMPLES];

	for (;;) {
		size_t n = audio_tap_read(in, FRAME_IN_SAMPLES);
		if (n == 0)
			return true;

		const uint8_t *planes[1] = {(const uint8_t *)in};
		uint8_t *out[MAX_AV_PLANES] = {0};
		uint32_t out_frames = 0;
		uint64_t ts_offset = 0;
		if (!audio_resampler_resample(resampler, out, &out_frames, &ts_offset, planes, (uint32_t)n))
			return true;
		if (out_frames == 0 || out_frames > FRAME_IN_SAMPLES)
			continue;

		uint64_t clock = os_gettime_ns();
		frame[0] = 0x01;
		memcpy(frame + 1, "OCAF", 4);
		put_u32(frame + 5, s->seq++);
		put_u32(frame + 9, (uint32_t)clock);
		put_u32(frame + 13, (uint32_t)(clock >> 32));
		put_u32(frame + 17, out_frames);
		memcpy(frame + 21, out[0], 2 * (size_t)out_frames);
		if (!write_all(s, frame, 21 + 2 * (size_t)out_frames))
			return false;
	}
}

static void tick(void)
{
	char text[SLC_CAPTION_MAX_BYTES + 1];
	int64_t duration_ms = 0;
	if (os_atomic_compare_swap_long(&reset_requested, 1, 0))
		slc_pipeline_reset(pipeline);
	if (slc_pipeline_tick(pipeline, now_ms(), text, sizeof(text), &duration_ms))
		caption_out_send(text, (double)duration_ms / 1000.0);
}

/* One engine run: until it exits, breaks the pipe or we are stopped. */
static void run_session(struct session *s)
{
	if (!launch(s))
		return;
	audio_tap_clear(); /* do not send a backlog of old audio */
	s->read_ov.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (!start_read(s))
		return;

	int64_t last_tick = now_ms();
	for (;;) {
		HANDLE wait[3] = {s->read_ov.hEvent, s->process, stop_event};
		DWORD w = WaitForMultipleObjects(3, wait, FALSE, LOOP_MS);
		if (w == WAIT_OBJECT_0 + 2)
			return;
		if (w == WAIT_OBJECT_0 + 1) {
			obs_log(LOG_WARNING, "speech engine exited");
			return;
		}
		if (w == WAIT_OBJECT_0) {
			DWORD got = 0;
			if (!GetOverlappedResult(s->pipe, &s->read_ov, &got, FALSE) || got == 0) {
				obs_log(LOG_WARNING, "speech engine connection closed");
				return;
			}
			handle_bytes(s, s->read_buf, got);
			if (!start_read(s))
				return;
		}
		if (s->listening && !send_audio(s))
			return;
		if (now_ms() - last_tick >= TICK_MS) {
			tick();
			last_tick = now_ms();
		}
	}
}

static void *engine_thread(void *arg)
{
	(void)arg;
	os_set_thread_name("slc-engine-client");

	struct resample_info src = {audio_tap_sample_rate(), AUDIO_FORMAT_FLOAT_PLANAR, SPEAKERS_MONO};
	struct resample_info dst = {16000, AUDIO_FORMAT_16BIT, SPEAKERS_MONO};
	resampler = audio_resampler_create(&dst, &src);
	pipeline = slc_pipeline_create(1);

	DWORD backoff = 1000;
	while (!stopping() && resampler) {
		set_state(ENGINE_STARTING);
		struct session s = {0};
		run_session(&s);
		bool reached_listening = s.listening;
		close_session(&s);
		if (stopping())
			break;

		long st = os_atomic_load_long(&state);
		if (st != ENGINE_MODEL_MISSING && st != ENGINE_NOT_FOUND)
			set_state(ENGINE_RESTARTING);
		if (reached_listening)
			backoff = 1000;
		WaitForSingleObject(stop_event, backoff);
		backoff = backoff * 2 > BACKOFF_MAX_MS ? BACKOFF_MAX_MS : backoff * 2;
	}

	/* Clear what is on screen; the pipeline is going away. */
	caption_out_send("", 0.5);
	slc_pipeline_destroy(pipeline);
	pipeline = NULL;
	audio_resampler_destroy(resampler);
	resampler = NULL;
	set_state(ENGINE_OFF);
	return NULL;
}

bool engine_client_init(void)
{
	stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
	return stop_event != NULL;
}

void engine_client_shutdown(void)
{
	engine_client_stop();
	if (stop_event)
		CloseHandle(stop_event);
	stop_event = NULL;
}

void engine_client_start(void)
{
	if (running || !stop_event)
		return;
	ResetEvent(stop_event);
	if (pthread_create(&thread, NULL, engine_thread, NULL) == 0)
		running = true;
	else
		obs_log(LOG_ERROR, "could not start the engine client thread");
}

void engine_client_stop(void)
{
	if (!running)
		return;
	SetEvent(stop_event);
	pthread_join(thread, NULL);
	running = false;
}

void engine_client_request_reset(void)
{
	os_atomic_set_long(&reset_requested, 1);
}

const char *engine_client_status_key(void)
{
	switch ((enum engine_state)os_atomic_load_long(&state)) {
	case ENGINE_STARTING:
		return "Engine.State.Starting";
	case ENGINE_LISTENING:
		return "Engine.State.Listening";
	case ENGINE_MODEL_MISSING:
		return "Engine.State.ModelMissing";
	case ENGINE_NOT_FOUND:
		return "Engine.State.NotFound";
	case ENGINE_RESTARTING:
		return "Engine.State.Restarting";
	case ENGINE_OFF:
	default:
		return "Engine.State.Off";
	}
}
