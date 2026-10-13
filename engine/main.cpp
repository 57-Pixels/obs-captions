/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * obs-captions-engine entry point. Today it only reports its protocol
 * version; the named-pipe server and ASR backends arrive in later phases.
 */

#include <cstdio>

#include "engine-api.h"

int main()
{
	std::printf("obs-captions-engine (protocol v%u)\n", obscap::engine::protocol_version);
	return 0;
}
