# SPDX-License-Identifier: GPL-2.0-or-later
#
# Pure C++17 caption library. No OBS headers or libraries, ever (AGENTS.md).
# Builds as part of the plugin, or standalone for tests:
#   cmake -S core -B build_core && cmake --build build_core && ctest --test-dir build_core -C Debug

cmake_minimum_required(VERSION 3.28...3.30)

project(simple-local-captions-core LANGUAGES CXX)

option(OBS_CAPTIONS_CORE_TESTS "Build simple-local-captions core unit tests" ${PROJECT_IS_TOP_LEVEL})

add_library(simple-local-captions-core STATIC)
target_compile_features(simple-local-captions-core PUBLIC cxx_std_17)
set_target_properties(simple-local-captions-core PROPERTIES CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)
target_include_directories(simple-local-captions-core PUBLIC include PRIVATE .)
if(MSVC)
  target_compile_options(simple-local-captions-core PRIVATE /W4 /WX /utf-8)
else()
  target_compile_options(simple-local-captions-core PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_sources(
  simple-local-captions-core
  PRIVATE
    include/slc/caption-update.h
    include/slc/config.h
    include/slc/events.h
    include/slc/pipeline.h
    include/slc/stats.h
    include/slc/version.h
    src/config.cpp
    src/pipeline.cpp
    src/version.cpp
    formatter/charset-608-tables.h
    formatter/charset-608.cpp
    formatter/charset-608.h
    formatter/transliterate.cpp
    formatter/transliterate.h
    formatter/utf8.cpp
    formatter/utf8.h
    formatter/wrap.cpp
    formatter/wrap.h
    filters/hallucination.cpp
    filters/hallucination.h
    filters/profanity.cpp
    filters/profanity.h
    filters/repeat.cpp
    filters/repeat.h
    filters/text-norm.cpp
    filters/text-norm.h
    filters/word-replace.cpp
    filters/word-replace.h
    scheduler/scheduler.cpp
    scheduler/scheduler.h
)

if(OBS_CAPTIONS_CORE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
