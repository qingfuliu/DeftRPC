message("=============TEST=============")
message(STATUS "Enable test module.")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCE LIST_DIRECTORIES false ./*.cpp)

foreach (filename IN LISTS TEST_SOURCE)
    get_filename_component(target ${filename} NAME_WLE)

    message(${target})

    add_executable(${target} ${filename})

    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src/include PUBLIC ${CMAKE_SOURCE_DIR}/include/log)

    target_link_libraries(${target}
            DeftRpc benchmark gtest_main)

    gtest_discover_tests(${target})
endforeach ()




