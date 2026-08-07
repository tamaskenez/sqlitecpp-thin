cmake_minimum_required(VERSION 3.25)
project(test-installed-lib)

enable_testing()

foreach(p IN LISTS CMAKE_PREFIX_PATH)
	message(STATUS "CMAKE_PREFIX_PATH: ${p}")
endforeach()

find_package(sqlitecpp-thin REQUIRED CONFIG)

add_executable(test-installed-lib test-installed-lib.cpp)
target_link_libraries(test-installed-lib PRIVATE
	sqlitecpp-thin::sqlitecpp-thin
)
add_test(NAME test-installed-lib COMMAND test-installed-lib)
