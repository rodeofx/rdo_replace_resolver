cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(replace-resolver VERSION 0.1.0 LANGUAGES CXX)

include(${USD_LOCATION}/pxrConfig.cmake)

set(CMAKE_INSTALL_RPATH ${USD_LOCATION}/lib)
set(USDPLUGIN_NAME replaceResolver)

add_subdirectory(src)

# Install etc folder
install(
  DIRECTORY etc
  DESTINATION ./${USDPLUGIN_NAME}
)
