# SPDX-License-Identifier: GPL-2.0-or-later

# Template plugin-support library (obs_log with "[simple-local-captions]" prefix). The
# template creates it from ./src in cmake/common/helpers_common.cmake; that
# check runs from the repository root, so it is set up here instead.
configure_file(src/plugin-support.c.in plugin-support.c @ONLY)
add_library(plugin-support STATIC)
target_sources(plugin-support PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/plugin-support.c" PUBLIC src/plugin-support.h)
target_include_directories(plugin-support PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

# The captions dock needs Qt and obs-frontend-api (both on in CMakePresets).
if(NOT ENABLE_QT OR NOT ENABLE_FRONTEND_API)
  message(FATAL_ERROR "simple-local-captions requires ENABLE_QT=ON and ENABLE_FRONTEND_API=ON")
endif()

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    src/plugin-main.c
    src/audio-tap.c
    src/audio-tap.h
    src/caption-out.c
    src/caption-out.h
    src/engine-client.c
    src/engine-client.h
    src/frontend.c
    src/frontend.h
)

if(ENABLE_QT)
  target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/ui/caption-dock.cpp src/ui/caption-dock.h src/ui/caption-dock.hpp)
endif()

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
