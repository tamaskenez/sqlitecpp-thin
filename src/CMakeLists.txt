add_subdirectory(sqlitecpp-thin)

if(PROJECT_IS_TOP_LEVEL)
	add_subdirectory(examples)
	add_subdirectory(tests)
endif()
