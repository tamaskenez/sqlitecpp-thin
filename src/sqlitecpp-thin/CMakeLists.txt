file(GLOB sources CONFIGURE_DEPENDS *.hpp *.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

find_package(SQLite3 REQUIRED)

set(do_install 0)

foreach(style exception expected)
	if(style STREQUAL expected AND NOT HAS_EXPECTED)
		continue()
	endif()

	set(target sqlitecpp-thin-${style})

	add_library(${target} ${sources})
	target_link_libraries(${target}
		PUBLIC
			SQLite::SQLite3
	)

	if(HAS_FORMAT)
        target_compile_definitions(${target}
            PRIVATE
                HAS_FORMAT
        )
    else()
        target_link_libraries(${target}
            PRIVATE
                fmt::fmt
        )
	endif()

	target_include_directories(${target}
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
			$<INSTALL_INTERFACE:include>
	)
	if(style STREQUAL exception)
		set(defs
			SQLITECPPTHIN_EXCEPTION=1
			SQLITECPPTHIN_EXPECTED=0
		)
	elseif(style STREQUAL expected)
		set(defs
			SQLITECPPTHIN_EXCEPTION=0
			SQLITECPPTHIN_EXPECTED=1
		)
	else()
		message(FATAL_ERROR)
	endif()

	target_compile_definitions(${target}
		PRIVATE
			${defs}
	)

	target_compile_features(${target} PUBLIC cxx_std_23)

	if(CMAKE_INSTALL_PREFIX)
		install(TARGETS ${target} EXPORT sqlitecpp-thin-targets)
		add_library(sqlitecpp-thin::${target} ALIAS ${target})
		install(FILES sqlite3-${style}.hpp
			DESTINATION include/sqlitecpp-thin
		)
		set(do_install 1)
	endif()
endforeach()

if(do_install)
	install(EXPORT sqlitecpp-thin-targets
		DESTINATION lib/cmake/sqlitecpp-thin
		NAMESPACE sqlitecpp-thin::
	)
	install(FILES sqlite3.hpp
		DESTINATION include/sqlitecpp-thin
	)
	if(HAS_FORMAT OR BUILD_SHARED_LIBS)
		set(FIND_FORMAT 0)
	else()
		set(FIND_FORMAT 1)
	endif()
	configure_file(sqlitecpp-thin-config.cmake.in sqlitecpp-thin-config.cmake @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sqlitecpp-thin-config.cmake
		DESTINATION lib/cmake/sqlitecpp-thin
	)
endif()
