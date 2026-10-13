# SPDX-License-Identifier: GPL-2.0-or-later
#
# Pure C++17 caption library. No OBS headers or libraries, ever (AGENTS.md).
# Builds as part of the plugin, or standalone for tests:
#   cmake -S core -B build_core && cmake --build build_core && ctest --test-dir build_core -C Debug

cmake_minimum_required(VERSION 3.28...3.30)

project(obs-captions-core LANGUAGES CXX)

option(OBS_CAPTIONS_CORE_TESTS "Build obs-captions core unit tests" ${PROJECT_IS_TOP_LEVEL})

add_library(obs-captions-core STATIC)
target_compile_features(obs-captions-core PUBLIC cxx_std_17)
set_target_properties(obs-captions-core PROPERTIES CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)
target_include_directories(obs-captions-core PUBLIC include)
if(MSVC)
  target_compile_options(obs-captions-core PRIVATE /W4 /WX /utf-8)
else()
  target_compile_options(obs-captions-core PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_sources(obs-captions-core PRIVATE include/obscap/version.h src/version.cpp)

if(OBS_CAPTIONS_CORE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
