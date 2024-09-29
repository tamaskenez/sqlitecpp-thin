source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES example.cpp)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE
	sqlitecpp-thin::sqlitecpp-thin-exception
)
