cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" OFF)
option(ENABLE_QT "Use Qt functionality" OFF)

include(compilerconfig)
include(defaults)
include(helpers)

# obs-captions: the OBS module lives in plugin/ so that its data/ directory
# resolves relative to plugin/CMakeLists.txt (see set_target_properties_plugin
# in cmake/windows/helpers.cmake).
add_subdirectory(core)
add_subdirectory(plugin)
