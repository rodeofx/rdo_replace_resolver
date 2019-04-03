set(USDPLUGIN_NAME helloResolver)

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


# Python bindings
if (PXR_ENABLE_PYTHON_SUPPORT)

set(USDPLUGIN_PYTHON_NAME _${USDPLUGIN_NAME})

string(REPLACE "_" "" LIBNAME ${USDPLUGIN_PYTHON_NAME})
string(SUBSTRING ${LIBNAME} 0 1 LIBNAME_FL)
string(TOUPPER ${LIBNAME_FL} LIBNAME_FL)
string(SUBSTRING ${LIBNAME} 1 -1 LIBNAME_SUFFIX)
set(MODULE_NAME "${LIBNAME_FL}${LIBNAME_SUFFIX}")

message(${USDPLUGIN_PYTHON_NAME})
message(${MODULE_NAME})

add_library(${USDPLUGIN_PYTHON_NAME}
    SHARED
    module.cpp
    moduleDeps.cpp
    wrapHelloResolver.cpp
    wrapHelloResolverContext.cpp
)

# On OSX "dylib" does not work to import python module
# TODO: windows support
set_target_properties(${USDPLUGIN_PYTHON_NAME} PROPERTIES SUFFIX ".so")

set_target_properties(${USDPLUGIN_PYTHON_NAME}
  PROPERTIES
      PREFIX ""
)
target_compile_definitions(${USDPLUGIN_PYTHON_NAME}
  PRIVATE
    MFB_PACKAGE_NAME=${USDPLUGIN_NAME}
    MFB_ALT_PACKAGE_NAME=${USDPLUGIN_NAME}
    MFB_PACKAGE_MODULE=rdo.Ar
)

target_link_libraries(${USDPLUGIN_PYTHON_NAME}
  ${USDPLUGIN_NAME}
)

set(PYTHON_PACKAGE_RELATIVE_PATH lib/python2.7/site-packages/rdo)
set(INSTALL_PYTHONPACKAGE_DIR  ${CMAKE_INSTALL_PREFIX}/${USDPLUGIN_NAME}/${PYTHON_PACKAGE_RELATIVE_PATH})
set(INSTALL_WRAPPER_DIR ${INSTALL_PYTHONPACKAGE_DIR}/${MODULE_NAME})

install(
  TARGETS ${USDPLUGIN_PYTHON_NAME}
  DESTINATION ${INSTALL_WRAPPER_DIR}
)

file(WRITE ${INSTALL_PYTHONPACKAGE_DIR}/__init__.py "''' helloResolver package '''")

install(
  FILES __init__.py
  DESTINATION ${INSTALL_WRAPPER_DIR}
)


endif (PXR_ENABLE_PYTHON_SUPPORT)
