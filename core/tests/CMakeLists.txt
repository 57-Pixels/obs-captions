# SPDX-License-Identifier: GPL-2.0-or-later

add_executable(obs-captions-core-tests)
target_sources(obs-captions-core-tests PRIVATE test-main.cpp test-version.cpp)
target_include_directories(obs-captions-core-tests PRIVATE ../third_party)
target_link_libraries(obs-captions-core-tests PRIVATE obs-captions-core)
if(MSVC)
  target_compile_options(obs-captions-core-tests PRIVATE /W4 /WX /utf-8)
else()
  target_compile_options(obs-captions-core-tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
target_compile_definitions(obs-captions-core-tests PRIVATE OBSCAP_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden")

add_test(NAME obs-captions-core-tests COMMAND obs-captions-core-tests)
