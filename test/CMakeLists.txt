include_directories(${CMAKE_SOURCE_DIR}/bot)

# Adds a cxx test. Suites are a list of test suites to be added to the test.  This macro
# assumes that the suite header and source names are identical and located in the same path. 
macro(add_cxxtest test_name suites libs)
	set (headers "")
	set (srcs "")
	foreach(suite ${suites})
		list(APPEND headers ${CMAKE_CURRENT_SOURCE_DIR}/${suite}.h)
		list(APPEND srcs ${CMAKE_CURRENT_SOURCE_DIR}/${suite}.cpp)
	endforeach(suite ${suites})
	add_custom_command(PRE_BUILD 
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${test_name}_runner.cpp
		COMMAND ${CXX_TEST_GEN} --error-printer -o ${CMAKE_CURRENT_BINARY_DIR}/${test_name}_runner.cpp ${headers}
		DEPENDS ${headers})
	add_executable(${test_name} ${CMAKE_CURRENT_BINARY_DIR}/${test_name}_runner.cpp ${srcs})
	target_link_libraries(${test_name}  ${libs} ${LEGACY_STDIO})
	add_test(${test_name} ${test_name})
endmacro(add_cxxtest)

add_subdirectory(goap)
add_cxxtest(sdk_test "ValveSDKTest" ":tier1.a;tier0")
