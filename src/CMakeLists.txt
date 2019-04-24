add_library(${USDPLUGIN_NAME}
    SHARED
    debugCodes.cpp
    debugCodes.h
    replaceResolver.cpp
    replaceResolver.h
    replaceResolverContext.cpp
    replaceResolverContext.h
    tokens.cpp
    tokens.h
)

target_include_directories(${USDPLUGIN_NAME}
    PRIVATE  
        ${PXR_INCLUDE_DIRS}
)

target_link_libraries(${USDPLUGIN_NAME}
    ar
    sdf
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

# Install plugInfo
install(
    FILES ${CMAKE_BINARY_DIR}/plugInfo.json
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${USDPLUGIN_NAME}/resources
)

# Python bindings
if (PXR_ENABLE_PYTHON_SUPPORT)

set(USDPLUGIN_PYTHON_NAME _${USDPLUGIN_NAME})

set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH}:${CMAKE_INSTALL_PREFIX}/${USDPLUGIN_NAME}/lib)

string(REPLACE "_" "" LIBNAME ${USDPLUGIN_PYTHON_NAME})
string(SUBSTRING ${LIBNAME} 0 1 LIBNAME_FL)
string(TOUPPER ${LIBNAME_FL} LIBNAME_FL)
string(SUBSTRING ${LIBNAME} 1 -1 LIBNAME_SUFFIX)
set(MODULE_NAME "${LIBNAME_FL}${LIBNAME_SUFFIX}")

add_library(${USDPLUGIN_PYTHON_NAME}
    SHARED
    module.cpp
    moduleDeps.cpp
    wrapReplaceResolver.cpp
    wrapReplaceResolverContext.cpp
    wrapTokens.cpp
)

target_include_directories(${USDPLUGIN_PYTHON_NAME}
    PRIVATE  
        ${PXR_INCLUDE_DIRS}
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
    MFB_PACKAGE_MODULE=rdo.${MODULE_NAME}
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

file(WRITE ${INSTALL_PYTHONPACKAGE_DIR}/__init__.py "''' replaceResolver package '''")

install(
    FILES __init__.py
    DESTINATION ${INSTALL_WRAPPER_DIR}
)

# Tests

set(_testPYTHONPATH "PYTHONPATH=${INSTALL_PYTHONPACKAGE_DIR}/..:${USD_LOCATION}/lib/python:$ENV{PYTHONPATH}")
set(_testPXR_PLUGINPATH_NAME "PXR_PLUGINPATH_NAME=${CMAKE_INSTALL_PREFIX}/${USDPLUGIN_NAME}/resources")

if(APPLE)
    # TODO: In theory it should not be necessary to set DYLD_LIBRARY_PATH.
    # I could not figure out how to make plugInfo work with python bindings on my Mac.
    set(_testDYLD_LIBRARY_PATH "DYLD_LIBRARY_PATH=${USD_LOCATION}/lib/:${CMAKE_INSTALL_PREFIX}/${USDPLUGIN_NAME}/lib")
endif()

install(
    FILES testenv/testAsArDefaultResolver.py
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test
)

add_test(
    NAME testAsArDefaultResolver
    COMMAND env ${_testDYLD_LIBRARY_PATH} ${_testPYTHONPATH} ${_testPXR_PLUGINPATH_NAME} python testAsArDefaultResolver.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test)

install(
    FILES testenv/testReplaceResolver.py
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test
)

add_test(
    NAME testReplaceResolver
    COMMAND env ${_testDYLD_LIBRARY_PATH} ${_testPYTHONPATH} ${_testPXR_PLUGINPATH_NAME} python testReplaceResolver.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test)

endif (PXR_ENABLE_PYTHON_SUPPORT)
