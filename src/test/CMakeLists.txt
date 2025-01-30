message("=============TEST=============")
message(STATUS "Enable test module.")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCE LIST_DIRECTORIES false ./*.cpp)

#foreach (filename IN LISTS TEST_SOURCE)
#    get_filename_component(target ${filename} NAME_WLE)
#
#    message(${target})
#
#    add_executable(${target} ${filename})
#
#    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src/include PUBLIC ${CMAKE_SOURCE_DIR}/include/log)
#
#    target_link_libraries(${target}
#            CLSN_HOOK CLSN_COROUTINE LOG benchmark
#            pthread CLSN_NET CLSN_SEVER CLSN_CLIENT
#            CLSN_DATASTRUCT CLSN_COMMON gtest_main)
#
#    gtest_discover_tests(${target})
#endforeach ()

enable_language(C ASM)

#aux_source_directory(. COROUTINE_SRC)

#list(APPEND COROUTINE_SRC asm/coctx_swap.S)

add_executable(test_coroutine
        ./coroutine/test_coroutine.cpp
        ../coroutine/asm/coctx_swap.S
        ../coroutine/Coroutine.cpp
        ../coroutine/CoroutineContext.cpp)

target_include_directories(test_coroutine PRIVATE ${CMAKE_SOURCE_DIR}/src/include PUBLIC ${CMAKE_SOURCE_DIR}/include/log)

target_link_libraries(test_coroutine
        LOG
        pthread CLSN_COMMON gtest_main)

add_dependencies(test_coroutine CLSN_COROUTINE LOG CLSN_COMMON)

gtest_discover_tests(test_coroutine)




