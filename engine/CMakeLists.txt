# SPDX-License-Identifier: GPL-2.0-or-later
#
# Separate speech-recognition process (ADR 0001). Must not link libobs.
# Installed with its DLLs into <plugin>/bin/64bit/engine/ so ONNX Runtime
# never sits next to the OBS plugin DLL.

include(cmake/sherpa-onnx.cmake)

add_executable(simple-local-captions-engine)
target_compile_features(simple-local-captions-engine PRIVATE cxx_std_17)
target_sources(
  simple-local-captions-engine
  PRIVATE
    main.cpp
    engine-api.h
    asr-sherpa.cpp
    asr-sherpa.h
    pipe-client.cpp
    pipe-client.h
    protocol.cpp
    protocol.h
    stability.cpp
    stability.h
    text-case.cpp
    text-case.h
)
target_link_libraries(simple-local-captions-engine PRIVATE sherpa_onnx::c_api)
if(MSVC)
  target_compile_options(simple-local-captions-engine PRIVATE /W4 /utf-8)
endif()

add_custom_command(
  TARGET simple-local-captions-engine
  POST_BUILD
  COMMAND
    "${CMAKE_COMMAND}" -E copy_if_different ${SHERPA_ONNX_RUNTIME_DLLS}
    "$<TARGET_FILE_DIR:simple-local-captions-engine>"
  COMMENT "Copy sherpa-onnx runtime DLLs next to the engine"
  VERBATIM
)

install(TARGETS simple-local-captions-engine RUNTIME DESTINATION "${CMAKE_PROJECT_NAME}/bin/64bit/engine")
install(FILES ${SHERPA_ONNX_RUNTIME_DLLS} DESTINATION "${CMAKE_PROJECT_NAME}/bin/64bit/engine")
