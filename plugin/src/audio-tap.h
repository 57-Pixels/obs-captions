/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Audio tap: the "Simple Local Captions" audio filter that streamers add to
 * their microphone (or any audio source). It passes audio through unchanged.
 *
 * Phase 2: the filter measures whether audio arrives and how loud it is, and
 * its properties hold the test-caption buttons and that status, so the
 * streamer can check the filter sits on the right source before speech
 * recognition exists.
 * Phase 3: the filter copies audio into a lock-free SPSC ring buffer for
 * engine-client.c.
 *
 * Rules (AGENTS.md "Danger zones"): the audio callback never blocks,
 * allocates, locks a contended mutex, logs or does I/O. It only reads the
 * samples and stores two atomics.
 */

#include <stdbool.h>

/* Registers the filter type with OBS (call from obs_module_load). */
bool audio_tap_init(void);
void audio_tap_shutdown(void);
