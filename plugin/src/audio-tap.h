/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Audio tap: the "Simple Local Captions" audio filter that streamers add to
 * their microphone (or any audio source). Audio passes through unchanged.
 *
 * Captions listen to one source at a time: the first filter instance owns
 * the tap. It mixes its audio to mono into a lock-free SPSC ring buffer
 * that engine-client.c drains, and starts the engine client; removing it
 * stops the engine. Every instance shows audio level, engine status and the
 * test-caption buttons in its properties.
 *
 * Rules (AGENTS.md "Danger zones"): the audio callback never blocks,
 * allocates, locks a contended mutex, logs or does I/O. It mixes into a
 * stack buffer, writes the ring buffer and stores two atomics.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Registers the filter type with OBS (call from obs_module_load). */
bool audio_tap_init(void);
void audio_tap_shutdown(void);

/* For engine-client.c (its thread only). Samples are mono float at
 * audio_tap_sample_rate(), OBS's output rate. */
uint32_t audio_tap_sample_rate(void);
size_t audio_tap_read(float *out, size_t max);
void audio_tap_clear(void);
