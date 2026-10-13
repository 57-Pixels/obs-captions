/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/*
 * Audio tap: copies audio from the selected OBS source into a lock-free
 * single-producer/single-consumer ring buffer for engine-client.c.
 *
 * Rules (AGENTS.md "Danger zones"): the OBS audio callback must never block,
 * allocate, lock a contended mutex or do I/O. It copies and returns.
 *
 * Stub: no behaviour until roadmap phase 3.
 */

#include <stdbool.h>

bool audio_tap_init(void);
void audio_tap_shutdown(void);
