set(CMAKE_IMPORT_FILE_VERSION 1)

set(PXR_INCLUDE_DIRS ${USD_LOCATION}/toolkit/include)

set(_DSOLIB_DIR "${USD_LOCATION}/dsolib")
set(_PYTHONLIB_DIR "${USD_LOCATION}/python/lib")
set(_PYTHONINCLUDE_DIR "${USD_LOCATION}/python/include/python2.7")

set(_LIB_BOOST_PYTHON "${_DSOLIB_DIR}/libhboost_python-mt.so")
set(_LIB_PYTHON "${_PYTHONLIB_DIR}/libpython2.7.so")
set(_LIB_TBB "${_DSOLIB_DIR}/libtbb.so")

set(_COMPONENTS
  arch
  tf
  js
  gf
  trace
  work
  vt
  plug
  ar
  sdf
)

set(LINK_INTERFACE_arch "dl;/usr/lib64/libm.so")
set(INCLUDE_DIRECTORIES_arch "${PXR_INCLUDE_DIRS}")

set(LINK_INTERFACE_tf "arch;${_LIB_PYTHON};${_LIB_BOOST_PYTHON};${_LIB_TBB}")
set(INCLUDE_DIRECTORIES_tf "${_PYTHONINCLUDE_DIR};${PXR_INCLUDE_DIRS}")

set(LINK_INTERFACE_js "tf")
set(INCLUDE_DIRECTORIES_js "${PXR_INCLUDE_DIRS}")

set(LINK_INTERFACE_gf "arch;tf")
set(INCLUDE_DIRECTORIES_gf "")

set(LINK_INTERFACE_trace "arch;js;tf;${_LIB_BOOST_PYTHON};${_LIB_TBB}")
set(INCLUDE_DIRECTORIES_trace "${PXR_INCLUDE_DIRS}")

set(LINK_INTERFACE_work "tf;trace;${_LIB_TBB}")
set(INCLUDE_DIRECTORIES_work "${PXR_INCLUDE_DIRS}")

# set(LINK_INTERFACE_vt "arch;tf;gf;trace;${_LIB_BOOST_PYTHON};${_LIB_TBB}")
set(LINK_INTERFACE_vt "tf;trace;${_LIB_BOOST_PYTHON};${_LIB_TBB}")
set(INCLUDE_DIRECTORIES_vt "${PXR_INCLUDE_DIRS}")

set(LINK_INTERFACE_plug "arch;tf;js;trace;work;${_LIB_BOOST_PYTHON};${_LIB_TBB}")
set(INCLUDE_DIRECTORIES_plug "${PXR_INCLUDE_DIRS}")

set(LINK_INTERFACE_ar "arch;tf;plug;vt;${_LIB_BOOST_PYTHON}")
set(INCLUDE_DIRECTORIES_ar ${PXR_INCLUDE_DIRS})

set(LINK_INTERFACE_sdf "arch;tf;gf;trace;vt;work;ar;${_LIB_BOOST_PYTHON}")
set(INCLUDE_DIRECTORIES_sdf ${PXR_INCLUDE_DIRS})

foreach(C ${_COMPONENTS})
  set(FILENAME "libpxr_${C}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set(LOCATION "${_DSOLIB_DIR}/${FILENAME}")

  add_library(${C} SHARED IMPORTED)

  # message("LIB: ${C}")
  # message("INTERFACE_INCLUDE_DIRECTORIES: ${INCLUDE_DIRECTORIES_${C}}")
  # message("IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE: ${LINK_INTERFACE_${C}}")
  # message("****")

  set_target_properties(${C}
    PROPERTIES
    INTERFACE_COMPILE_DEFINITIONS "PXR_PYTHON_ENABLED=1"
    INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIRECTORIES_${C}}"
    IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "${LINK_INTERFACE_${C}}"
    IMPORTED_LOCATION_RELEASE ${LOCATION}
    IMPORTED_SONAME_RELEASE ${FILENAME}
  )

  set_property(TARGET ${C} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)

  set(PXR_${C}_LIBRARY ${LOCATION})

  list(APPEND PXR_LIBRARIES ${C})
endforeach()

unset(_COMPONENTS)
unset(_LIB_TBB)
unset(_LIB_PYTHON)
unset(_LIB_BOOST_PYTHON)
unset(_PYTHONINCLUDE_DIR)
unset(_PYTHONLIB_DIR)
unset(_DSOLIB_DIR)
