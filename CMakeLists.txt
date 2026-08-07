cmake_minimum_required(VERSION 3.25)

project(sqlitecpp-thin)

# Conan 2.x CMakeDeps creates per-config targets (CONAN_LIB::*_RELEASE/_DEBUG) with only
# IMPORTED_LOCATION_<CONFIG> set. CMake 4.x validates all imported targets at configure
# time, erroring on missing locations for other configs. Setting CMP0111 OLD turns this
# back into a warning — Conan's config files select the correct variant at link time.
cmake_policy(SET CMP0111 OLD)

set(CMAKE_DEBUG_POSTFIX -debug)

if(PROJECT_IS_TOP_LEVEL)
	if(NOT CMAKE_PREFIX_PATH)
		set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/_d")
	endif()
	foreach(p IN LISTS CMAKE_PREFIX_PATH)
		message(STATUS "CMAKE_PREFIX_PATH: ${p}")
	endforeach()
	if(NOT CMAKE_CXX_STANDARD)
		set(CMAKE_CXX_STANDARD 23)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)
	endif()
	enable_testing()
	set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
	find_package(GTest REQUIRED CONFIG)
endif()

if(NOT TARGET SQLite3::SQLite3)
	find_package(SQLite3 REQUIRED CONFIG)
endif()

include(cmake/warnings_clang.cmake)
include(cmake/warnings_gcc.cmake)
include(cmake/warnings_msvc.cmake)

add_subdirectory(src)
