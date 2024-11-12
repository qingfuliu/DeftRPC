cmake_minimum_required(VERSION 3.25)
project(DeftRPC)

add_subdirectory(./common)

if (ENABLE_TEST)
    add_subdirectory(./test)
endif ()

add_subdirectory(./log)
add_subdirectory(./coroutine)
add_subdirectory(./net)
add_subdirectory(./hook)
add_subdirectory(./dataStruct)
add_subdirectory(./hash)
add_subdirectory(./rpc)
#target_include_directories(CLSN_COROUTINE  PRIVATE ${CMAKE_SOURCE_DIR}/include/)



