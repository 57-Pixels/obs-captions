# SPDX-License-Identifier: GPL-2.0-or-later
#
# Separate speech-recognition process (ADR 0001). Must not link libobs.

add_executable(simple-local-captions-engine)
target_compile_features(simple-local-captions-engine PRIVATE cxx_std_17)
target_sources(simple-local-captions-engine PRIVATE main.cpp engine-api.h)
