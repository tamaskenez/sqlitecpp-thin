file(GLOB sources CONFIGURE_DEPENDS *.hpp *.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

add_library(sqlitecpp-thin ${sources})
add_library(sqlitecpp-thin::sqlitecpp-thin ALIAS sqlitecpp-thin)

target_link_libraries(sqlitecpp-thin
	PUBLIC
		SQLite::SQLite3
)

target_include_directories(sqlitecpp-thin
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_compile_features(sqlitecpp-thin PUBLIC cxx_std_23)

if(CMAKE_INSTALL_PREFIX)
	install(TARGETS sqlitecpp-thin EXPORT sqlitecpp-thin-targets)
	install(FILES include/sqlitecpp-thin/sqlite3.hpp
		DESTINATION include/sqlitecpp-thin
	)
	install(EXPORT sqlitecpp-thin-targets
		DESTINATION lib/cmake/sqlitecpp-thin
		NAMESPACE sqlitecpp-thin::
	)
	configure_file(sqlitecpp-thin-config.cmake.in sqlitecpp-thin-config.cmake @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sqlitecpp-thin-config.cmake
		DESTINATION lib/cmake/sqlitecpp-thin
	)
endif()
