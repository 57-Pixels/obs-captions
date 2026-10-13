/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/*
 * Types shared by the engine process and its ASR backends. The wire format
 * between plugin and engine is specified in protocol/PROTOCOL.md; this header
 * only mirrors its constants so both sides agree.
 *
 * Stub: no ASR backend exists yet (roadmap phase 4).
 */

#include <cstdint>

namespace slc::engine {

constexpr std::uint32_t protocol_version = 0;
constexpr std::uint32_t sample_rate_hz = 16000;
constexpr std::uint32_t channels = 1;

enum class HypothesisKind { partial, final };

} // namespace slc::engine
