# SPDX-License-Identifier: GPL-2.0-or-later
#
# Unit tests for the engine's pure modules (no sherpa-onnx, no pipes).
# Standalone: cmake -S engine/tests -B build_engine_tests

cmake_minimum_required(VERSION 3.28...3.30)

project(simple-local-captions-engine-tests LANGUAGES CXX)

add_executable(simple-local-captions-engine-tests)
target_compile_features(simple-local-captions-engine-tests PRIVATE cxx_std_17)
target_sources(
  simple-local-captions-engine-tests
  PRIVATE test-main.cpp test-engine.cpp ../protocol.cpp ../stability.cpp ../text-case.cpp
)
target_include_directories(simple-local-captions-engine-tests PRIVATE .. ../../core/third_party)
if(MSVC)
  target_compile_options(simple-local-captions-engine-tests PRIVATE /W4 /WX /utf-8)
else()
  target_compile_options(simple-local-captions-engine-tests PRIVATE -Wall -Wextra -Werror)
endif()

enable_testing()
add_test(NAME simple-local-captions-engine-tests COMMAND simple-local-captions-engine-tests)
