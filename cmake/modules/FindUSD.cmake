if(USE_HOUDINI_USD)
  include(pxrConfigHoudini)
elseif(EXISTS ${USD_LOCATION}/pxrConfig.cmake)
  include(${USD_LOCATION}/pxrConfig.cmake)
  set(USD_PLUGINS_DIR ${USD_LOCATION}/plugin/usd)
  set(USD_PYTHONPATH ${USD_LOCATION}/lib/python)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(USD
  DEFAULT_MSG
  PXR_INCLUDE_DIRS
  PXR_LIBRARIES
)
