add_subdirectory(sqlitecpp-thin)

if(BUILD_TESTING)
	add_subdirectory(examples)
	add_subdirectory(tests)
endif()
