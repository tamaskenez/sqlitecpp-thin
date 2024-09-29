source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES int_storage_test.cpp)

add_executable(example int_storage_test.cpp)
target_link_libraries(example PRIVATE
	sqlitecpp-thin::sqlitecpp-thin-exception
)
