set(APP_NAME testUsdApp)

add_executable(${APP_NAME} main.cpp)

target_compile_features(${APP_NAME}
    PUBLIC
        cxx_auto_type
        cxx_lambdas
        cxx_nullptr
        cxx_range_for)

target_link_libraries(${APP_NAME} PUBLIC 
	ar
	usd
	usdHelloResolver)

install(TARGETS ${APP_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
