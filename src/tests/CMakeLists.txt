file(GLOB sources CONFIGURE_DEPENDS *.hpp *.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

add_executable(tests ${sources})
target_link_libraries(tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    sqlitecpp-thin::sqlitecpp-thin
)
target_precompile_headers(tests PRIVATE pch.hpp)
add_test(NAME tests COMMAND tests)
