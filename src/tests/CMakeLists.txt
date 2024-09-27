if(HAS_EXPECTED)
    set(sources tests.cpp)
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
else()
    message(STATUS "Tests skipped due to no <expected> found.")
    add_executable(empty_test empty_test.cpp)
    target_link_libraries(empty_test PRIVATE
        GTest::gtest
        GTest::gtest_main
    )
    target_precompile_headers(empty_test PRIVATE <gtest/gtest.h>)
    add_test(NAME empty_test COMMAND empty_test)
endif()
