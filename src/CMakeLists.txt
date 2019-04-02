set(USDPLUGIN_NAME usdHelloResolver)

add_library(${USDPLUGIN_NAME}
  SHARED
  resolver.cpp
  resolver.h
)

set_target_properties(${USDPLUGIN_NAME} PROPERTIES PREFIX "")

target_compile_features(${USDPLUGIN_NAME}
    PUBLIC
        cxx_auto_type
        cxx_lambdas
        cxx_nullptr
        cxx_range_for)

target_link_libraries(${USDPLUGIN_NAME}
  ar
)

install(
  TARGETS ${USDPLUGIN_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${USDPLUGIN_NAME}/lib
)

install(
  FILES plugInfo.json
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${USDPLUGIN_NAME}/resources
)
