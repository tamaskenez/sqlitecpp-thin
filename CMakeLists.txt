cmake_minimum_required(VERSION 3.25)

project(sqlitecpp-thin)

if(NOT DEFINED CMAKE_FIND_PACKAGE_PREFER_CONFIG)
	set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
endif()

if(NOT CMAKE_INSTALL_PREFIX)
	message(WARNING "CMAKE_INSTALL_PREFIX is empty, skipping install.")
endif()
set(CMAKE_DEBUG_POSTFIX -debug)

if(PROJECT_IS_TOP_LEVEL)
	if(NOT CMAKE_PREFIX_PATH)
		set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/id")
	endif()
endif()

if(NOT CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("expected" HAS_EXPECTED)
CHECK_INCLUDE_FILE_CXX("format" HAS_FORMAT)

if(NOT HAS_EXPECTED)
	message(STATUS "The <expected> header not found, std::expected-based error handling is disabled.")
endif()

if(NOT HAS_FORMAT)
	message(STATUS "The <format> header not found, using fmt::format instead.")
	find_package(fmt REQUIRED)
endif()

if(BUILD_TESTING)
	enable_testing()
	find_package(GTest REQUIRED)
endif()
find_package(SQLite3 REQUIRED)

set(WARNINGS_AS_ERRORS 1)
include(cpp_warnings.cmake)
add_compile_options(${PROJECT_WARNING_FLAGS})

add_subdirectory(src)
