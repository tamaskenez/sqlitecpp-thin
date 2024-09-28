cmake_minimum_required(VERSION 3.25)
project(test-installed-lib)

message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
list(LENGTH CMAKE_PREFIX_PATH l)
message(STATUS "length: ${l}")
foreach(a IN LISTS CMAKE_PREFIX_PATH)
	message(STATUS ${a})
endforeach()
message(STATUS "----")

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

enable_testing()

find_package(sqlitecpp-thin REQUIRED CONFIG)

add_executable(test-installed-lib-exception test-installed-lib-exception.cpp)
target_link_libraries(test-installed-lib-exception PRIVATE
	sqlitecpp-thin::sqlitecpp-thin-exception
)
add_test(NAME test-installed-lib-exception COMMAND test-installed-lib-exception)

if(TARGET sqlitecpp-thin-expected)
	add_executable(test-installed-lib-expected test-installed-lib-expected.cpp)
	target_link_libraries(test-installed-lib-expected PRIVATE
		sqlitecpp-thin-expected
	)
	add_test(NAME test-installed-lib-expected COMMAND test-installed-lib-expected)
endif()
