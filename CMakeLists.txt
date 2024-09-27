cmake_minimum_required(VERSION 3.25)

project(sqlitecpp-thin)

if(PROJECT_IS_TOP_LEVEL)
	if(NOT CMAKE_PREFIX_PATH)
		set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/id")
	endif()
endif()

if(NOT CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(BUILD_TESTING)
	enable_testing()
	find_package(GTest REQUIRED CONFIG)
endif()
find_package(SQLite3 REQUIRED CONFIG)

set(WARNINGS_AS_ERRORS 1)
include(cpp_warnings.cmake)
add_compile_options(${PROJECT_WARNING_FLAGS})

add_subdirectory(src)
