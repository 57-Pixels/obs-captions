/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "pipe-client.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "protocol.h"

namespace slc::engine {

PipeClient::~PipeClient()
{
	if (handle)
		CloseHandle(static_cast<HANDLE>(handle));
}

bool PipeClient::connect(const std::string &name, unsigned timeout_ms)
{
	std::string path = "\\\\.\\pipe\\" + name;
	ULONGLONG deadline = GetTickCount64() + timeout_ms;
	for (;;) {
		HANDLE h =
			CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
		if (h != INVALID_HANDLE_VALUE) {
			handle = h;
			return true;
		}
		if (GetTickCount64() >= deadline)
			return false;
		if (GetLastError() == ERROR_PIPE_BUSY)
			WaitNamedPipeA(path.c_str(), 200);
		else
			Sleep(50);
	}
}

size_t PipeClient::read(uint8_t *data, size_t size)
{
	DWORD got = 0;
	if (!ReadFile(static_cast<HANDLE>(handle), data, static_cast<DWORD>(size), &got, nullptr))
		return 0;
	return got;
}

bool PipeClient::write_json(const std::string &line)
{
	std::string framed;
	framed.reserve(line.size() + 2);
	framed += static_cast<char>(frame_json);
	framed += line;
	framed += '\n';
	DWORD written = 0;
	return WriteFile(static_cast<HANDLE>(handle), framed.data(), static_cast<DWORD>(framed.size()), &written,
			 nullptr) &&
	       written == framed.size();
}

} // namespace slc::engine
