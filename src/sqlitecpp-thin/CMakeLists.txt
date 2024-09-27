file(GLOB sources CONFIGURE_DEPENDS *.hpp *.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

find_package(SQLite3 REQUIRED)

foreach(style exception expected)
	if(style STREQUAL expected AND NOT HAS_EXPECTED)
		continue()
	endif()

	add_library(sqlitecpp-thin-${style} ${sources})
	target_link_libraries(sqlitecpp-thin-${style}
		PUBLIC
			SQLite::SQLite3
	)

	if(HAS_FORMAT)
        target_compile_definitions(sqlitecpp-thin-${style}
            PRIVATE
                HAS_FORMAT
        )
    else()
        target_link_libraries(sqlitecpp-thin-${style}
            PRIVATE
                fmt::fmt
        )
	endif()

	target_include_directories(sqlitecpp-thin-${style}
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

	target_compile_definitions(sqlitecpp-thin-${style}
		PRIVATE
			${defs}
	)

	add_library(sqlitecpp-thin::sqlitecpp-thin-${style} ALIAS sqlitecpp-thin-${style})
endforeach()
