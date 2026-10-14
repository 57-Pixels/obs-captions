# SPDX-License-Identifier: GPL-2.0-or-later

add_executable(simple-local-captions-core-tests)
target_sources(
  simple-local-captions-core-tests
  PRIVATE
    test-main.cpp
    test-version.cpp
    test-golden.cpp
    test-transliterate.cpp
    test-wrap.cpp
    test-filters.cpp
    test-scheduler.cpp
    test-c-api.cpp
)
target_include_directories(simple-local-captions-core-tests PRIVATE ../third_party ..)
target_link_libraries(simple-local-captions-core-tests PRIVATE simple-local-captions-core)
if(MSVC)
  target_compile_options(simple-local-captions-core-tests PRIVATE /W4 /WX /utf-8)
else()
  target_compile_options(simple-local-captions-core-tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
target_compile_definitions(simple-local-captions-core-tests PRIVATE SLC_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden")

add_test(NAME simple-local-captions-core-tests COMMAND simple-local-captions-core-tests)
