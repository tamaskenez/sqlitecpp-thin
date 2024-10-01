file(GLOB sources_expected CONFIGURE_DEPENDS expected/*.cpp)
file(GLOB sources_exception CONFIGURE_DEPENDS exception/*.cpp)
file(GLOB sources_combined CONFIGURE_DEPENDS combined/*.cpp)

foreach(style "expected" "exception")
    if(NOT HAS_EXPECTED AND style STREQUAL "expected")
        continue()
    endif()
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources_${style}} ${sources_combined})
    add_executable(tests-${style} ${sources_${style}} ${sources_combined})
    target_link_libraries(tests-${style} PRIVATE
        GTest::gtest
        GTest::gtest_main
        sqlitecpp-thin::sqlitecpp-thin-${style}
    )
    target_precompile_headers(tests-${style} PRIVATE ${style}/pch.hpp)
    add_test(NAME tests-${style} COMMAND tests-${style})
endforeach()
