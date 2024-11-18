message("=============TEST=============")
message(STATUS "Enable test module.")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

include(GoogleTest)

aux_source_directory(./ TEST_SOURCE)

foreach (filename IN LISTS TEST_SOURCE)
    #foreach (filename ${TEST_SOURCE})
    get_filename_component(target ${filename} NAME_WLE)

    message(${target})

    add_executable(${target} ${filename})

    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src/include PUBLIC ${CMAKE_SOURCE_DIR}/include/log)

    target_link_libraries(${target}
            CLSN_HOOK CLSN_COROUTINE LOG benchmark
            pthread CLSN_NET CLSN_SEVER CLSN_CLIENT
            CLSN_DATASTRUCT CLSN_RPC CLSN_COMMON gtest_main)

    gtest_discover_tests(${target})
endforeach ()




