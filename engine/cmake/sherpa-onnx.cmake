# SPDX-License-Identifier: GPL-2.0-or-later
#
# Fetches the prebuilt sherpa-onnx C API for Windows x64 (with ONNX Runtime)
# from its GitHub release, pinned by SHA-256, and defines sherpa_onnx::c_api.
# The header is vendored in engine/third_party/sherpa-onnx at the same
# version. See docs/DEPENDENCIES.md.

set(SHERPA_ONNX_VERSION 1.13.8)
set(_sherpa_name "sherpa-onnx-v${SHERPA_ONNX_VERSION}-win-x64-shared-MD-Release-no-tts-lib")
set(
  _sherpa_url
  "https://github.com/k2-fsa/sherpa-onnx/releases/download/v${SHERPA_ONNX_VERSION}/${_sherpa_name}.tar.bz2"
)
set(_sherpa_sha256 b90992b888710715d613a4fedcd75d5b4db294ef64f3194e4f77ea0a8b387441)
set(_sherpa_dir "${CMAKE_SOURCE_DIR}/.deps/sherpa")
set(_sherpa_archive "${_sherpa_dir}/${_sherpa_name}.tar.bz2")
set(SHERPA_ONNX_LIB_DIR "${_sherpa_dir}/${_sherpa_name}/lib")

if(NOT EXISTS "${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-c-api.lib")
  message(STATUS "Downloading sherpa-onnx ${SHERPA_ONNX_VERSION}")
  file(DOWNLOAD "${_sherpa_url}" "${_sherpa_archive}" EXPECTED_HASH SHA256=${_sherpa_sha256} STATUS _status)
  list(GET _status 0 _code)
  if(NOT _code EQUAL 0)
    message(FATAL_ERROR "Downloading sherpa-onnx failed: ${_status}")
  endif()
  file(ARCHIVE_EXTRACT INPUT "${_sherpa_archive}" DESTINATION "${_sherpa_dir}")
endif()

add_library(sherpa_onnx_c_api SHARED IMPORTED)
set_target_properties(
  sherpa_onnx_c_api
  PROPERTIES
    IMPORTED_IMPLIB "${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-c-api.lib"
    IMPORTED_LOCATION "${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-c-api.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../third_party/sherpa-onnx"
)
add_library(sherpa_onnx::c_api ALIAS sherpa_onnx_c_api)

# DLLs the engine needs at run time, copied next to the executable.
set(
  SHERPA_ONNX_RUNTIME_DLLS
  "${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-c-api.dll"
  "${SHERPA_ONNX_LIB_DIR}/onnxruntime.dll"
  "${SHERPA_ONNX_LIB_DIR}/onnxruntime_providers_shared.dll"
)
