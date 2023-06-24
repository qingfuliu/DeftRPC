cmake_minimum_required(VERSION 3.25)
project(DeftRPC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (CMAKE_BUILD_TYPE)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("======Debug======")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -O0")
        add_compile_definitions(DEBUG)
    else ()
        message("======Release======")
        add_compile_definitions(RELEASE)
    endif ()
else ()
    message("======Release======")
    add_compile_definitions(RELEASE)
endif ()


if ()
    add_compile_definitions(DEBUG)
else ()

endif ()

add_executable(DeftRPC main.cpp)


add_subdirectory(./src)
add_subdirectory(./example)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
