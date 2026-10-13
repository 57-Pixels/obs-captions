/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/*
 * Frontend glue: reacts to stream start/stop (obs-frontend-api events),
 * re-arms caption state on stream start and runs health checks that feed
 * plain-language status to the dock.
 *
 * Stub: no behaviour until roadmap phase 2.
 */

#include <stdbool.h>

bool frontend_init(void);
void frontend_shutdown(void);
