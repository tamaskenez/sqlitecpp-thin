cmake_minimum_required(VERSION 3.25)
project(test-installed-lib)

enable_testing()

if(NOT CMAKE_PREFIX_PATH)
	cmake_path(GET PROJECT_SOURCE_DIR PARENT_PATH project_source_dir_parent)
	foreach(d _d _i)
		cmake_path(APPEND project_source_dir_parent ${d} OUTPUT_VARIABLE p)
		list(APPEND CMAKE_PREFIX_PATH ${p})
	endforeach()
endif()

foreach(p IN LISTS CMAKE_PREFIX_PATH)
	message(STATUS "CMAKE_PREFIX_PATH: ${p}")
endforeach()

find_package(sqlitecpp-thin REQUIRED CONFIG)

add_executable(test-installed-lib test-installed-lib.cpp)
target_link_libraries(test-installed-lib PRIVATE
	sqlitecpp-thin::sqlitecpp-thin
)
add_test(NAME test-installed-lib COMMAND test-installed-lib)
