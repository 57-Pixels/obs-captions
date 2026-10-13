/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * simple-local-captions-engine entry point. Today it only reports its protocol
 * version; the named-pipe server and ASR backends arrive in later phases.
 */

#include <cstdio>

#include "engine-api.h"

int main()
{
	std::printf("simple-local-captions-engine (protocol v%u)\n", slc::engine::protocol_version);
	return 0;
}
