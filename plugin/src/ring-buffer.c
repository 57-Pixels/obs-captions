/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "ring-buffer.h"

/* bzalloc, bfree: libobs/util/bmem.h */
#include <util/bmem.h>
/* os_atomic_load_long, os_atomic_set_long, os_atomic_inc_long:
 * libobs/util/threading.h (full barriers on Windows) */
#include <util/threading.h>

/*
 * Positions only grow; they are compared as unsigned so the difference is
 * correct even after the 32-bit counters wrap.
 */
static unsigned long load_pos(volatile long *pos)
{
	return (unsigned long)os_atomic_load_long(pos);
}

bool ring_buffer_init(struct ring_buffer *rb, size_t capacity_pow2)
{
	if (capacity_pow2 == 0 || (capacity_pow2 & (capacity_pow2 - 1)) != 0)
		return false;
	rb->data = bzalloc(capacity_pow2 * sizeof(float));
	rb->mask = capacity_pow2 - 1;
	rb->write_pos = 0;
	rb->read_pos = 0;
	rb->dropped = 0;
	return true;
}

void ring_buffer_free(struct ring_buffer *rb)
{
	bfree(rb->data);
	rb->data = NULL;
}

size_t ring_buffer_write(struct ring_buffer *rb, const float *samples, size_t count)
{
	unsigned long w = load_pos(&rb->write_pos);
	unsigned long r = load_pos(&rb->read_pos);
	size_t used = (size_t)(w - r);
	size_t space = rb->mask + 1 - used;
	size_t n = count < space ? count : space;

	for (size_t i = 0; i < n; i++)
		rb->data[(w + i) & rb->mask] = samples[i];
	os_atomic_set_long(&rb->write_pos, (long)(w + (unsigned long)n));

	if (n < count)
		os_atomic_set_long(&rb->dropped, os_atomic_load_long(&rb->dropped) + (long)(count - n));
	return n;
}

size_t ring_buffer_read(struct ring_buffer *rb, float *out, size_t max)
{
	unsigned long r = load_pos(&rb->read_pos);
	unsigned long w = load_pos(&rb->write_pos);
	size_t avail = (size_t)(w - r);
	size_t n = max < avail ? max : avail;

	for (size_t i = 0; i < n; i++)
		out[i] = rb->data[(r + i) & rb->mask];
	os_atomic_set_long(&rb->read_pos, (long)(r + (unsigned long)n));
	return n;
}

void ring_buffer_clear(struct ring_buffer *rb)
{
	os_atomic_set_long(&rb->read_pos, os_atomic_load_long(&rb->write_pos));
}
