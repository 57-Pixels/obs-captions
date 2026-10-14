/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Client end of the plugin's named pipe (Windows). The engine uses one
 * thread with blocking I/O: read audio, run the model, write results.
 */

#include <cstddef>
#include <cstdint>
#include <string>

namespace slc::engine {

class PipeClient {
public:
	~PipeClient();

	/* Connects to \\.\pipe\<name>, waiting up to timeout_ms. */
	bool connect(const std::string &name, unsigned timeout_ms);
	/* Reads up to size bytes; returns 0 when the plugin closed the pipe. */
	size_t read(uint8_t *data, size_t size);
	/* Writes one JSON line framed as protocol type 0x02. */
	bool write_json(const std::string &line);

private:
	void *handle = nullptr; /* HANDLE */
};

} // namespace slc::engine
