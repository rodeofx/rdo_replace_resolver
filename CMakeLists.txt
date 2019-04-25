cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(replace-resolver VERSION 0.1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/defaults)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

enable_testing()

option(PXR_ENABLE_PYTHON_SUPPORT "Build Python wrapper and Python based tests" ON)
option(USE_HOUDINI_USD "Build against Houdini USD." OFF)

find_package(USD REQUIRED)

set(USDPLUGIN_NAME replaceResolver)

add_subdirectory(src)

# Install etc folder
install(
  DIRECTORY etc
  DESTINATION .
)
