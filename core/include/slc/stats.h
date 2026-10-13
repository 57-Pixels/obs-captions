/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace slc {

enum class SuppressReason {
	repeat,        /* (near-)duplicate final, see filters/repeat.h */
	hallucination, /* isolated silence artifact, see filters/hallucination.h */
	superseded,    /* display text replaced before it could be sent */
	count,
};

const char *suppress_reason_name(SuppressReason reason);

struct Stats {
	uint64_t emitted = 0;
	std::array<uint64_t, static_cast<size_t>(SuppressReason::count)> suppressed{};
	/* Characters dropped because CEA-608 cannot show them. */
	uint64_t stripped_chars = 0;
	/* now - end time of the newest text on screen at the last tick; 0 when
	 * the screen is empty. */
	int64_t lag_ms = 0;

	uint64_t suppressed_by(SuppressReason reason) const { return suppressed[static_cast<size_t>(reason)]; }
};

} // namespace slc
