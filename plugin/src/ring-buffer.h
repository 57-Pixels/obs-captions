/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Lock-free single-producer/single-consumer ring buffer of float samples.
 * The producer is the OBS audio thread (audio-tap.c), the consumer the
 * engine-client thread. No locks, no allocation after init: safe for the
 * audio callback (AGENTS.md "Danger zones").
 *
 * When full, new samples are dropped (and counted) rather than blocking the
 * audio thread.
 */

#include <stdbool.h>
#include <stddef.h>

struct ring_buffer {
	float *data;
	size_t mask;             /* capacity - 1; capacity is a power of two */
	volatile long write_pos; /* producer only writes */
	volatile long read_pos;  /* consumer only writes */
	volatile long dropped;   /* samples lost because the buffer was full */
};

/* capacity_pow2: number of samples, must be a power of two. */
bool ring_buffer_init(struct ring_buffer *rb, size_t capacity_pow2);
void ring_buffer_free(struct ring_buffer *rb);

/* Producer. Returns the number of samples written. */
size_t ring_buffer_write(struct ring_buffer *rb, const float *samples, size_t count);
/* Consumer. Returns the number of samples read (0 if empty). */
size_t ring_buffer_read(struct ring_buffer *rb, float *out, size_t max);
/* Consumer: discard everything currently buffered. */
void ring_buffer_clear(struct ring_buffer *rb);
