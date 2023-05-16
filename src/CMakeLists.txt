cmake_minimum_required(VERSION 3.25)
project(DeftRPC)


add_subdirectory(./test)
add_subdirectory(./log)
add_subdirectory(./coroutine)
add_subdirectory(./net)
add_subdirectory(./hook)
add_subdirectory(./dataStruct)
add_subdirectory(./hash)
#target_include_directories(CLSN_COROUTINE  PRIVATE ${CMAKE_SOURCE_DIR}/include/)



