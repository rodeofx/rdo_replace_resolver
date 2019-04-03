set(USDPLUGIN_NAME usdHelloResolver)

add_library(${USDPLUGIN_NAME}
  SHARED
  helloResolver.cpp
  helloResolver.h
  helloResolverContext.cpp
  helloResolverContext.h
)

target_link_libraries(${USDPLUGIN_NAME}
  ar
)

target_include_directories(${USDPLUGIN_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

set_target_properties(${USDPLUGIN_NAME} PROPERTIES PREFIX "")

target_compile_features(${USDPLUGIN_NAME}
    PUBLIC
        cxx_auto_type
        cxx_lambdas
        cxx_nullptr
        cxx_range_for)

set(libraryName ${USDPLUGIN_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

configure_file(
  plugInfo.json.in 
  ${CMAKE_BINARY_DIR}/plugInfo.json 
  @ONLY)

# Install library
install(
  TARGETS ${USDPLUGIN_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${USDPLUGIN_NAME}/lib
)

# Install headers
install(
  FILES helloResolverContext.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${USDPLUGIN_NAME}/include/${USDPLUGIN_NAME}
)

# Install plugInfo
install(
  FILES ${CMAKE_BINARY_DIR}/plugInfo.json
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${USDPLUGIN_NAME}/resources
)
