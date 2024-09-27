file(GLOB sources CONFIGURE_DEPENDS *.hpp *.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

foreach(style exception expected)
    add_executable(tests-${style} ${sources})
    target_link_libraries(tests-${style} PRIVATE
        GTest::gtest
        GTest::gtest_main
        sqlitecpp-thin::sqlitecpp-thin-${style}
    )
    target_precompile_headers(tests-${style} PRIVATE <gtest/gtest.h>)
    target_compile_definitions(tests-${style} PRIVATE
        SQLITECPPTHIN_HEADER=\"sqlitecpp-thin/sqlite3-${style}.hpp\"
    )
    add_test(NAME tests-${style} COMMAND tests-${style})
endforeach()
