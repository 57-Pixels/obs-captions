# SPDX-License-Identifier: GPL-2.0-or-later
#
# Separate speech-recognition process (ADR 0001). Must not link libobs.

add_executable(obs-captions-engine)
target_compile_features(obs-captions-engine PRIVATE cxx_std_17)
target_sources(obs-captions-engine PRIVATE main.cpp engine-api.h)
