cmake_minimum_required(VERSION 3.25)

if (USE_CLANG STREQUAL "True")
    execute_process(COMMAND which clang OUTPUT_VARIABLE CLANG_LOCATION)
    string(LENGTH ${CLANG_LOCATION} CLANG_LOCATION_LENGTH)

    execute_process(COMMAND which clang++ OUTPUT_VARIABLE CLANG_CPP_LOCATION)
    string(LENGTH ${CLANG_CPP_LOCATION} CLANG_CPP_LOCATION_LENGTH)

    if (${CLANG_CPP_LOCATION_LENGTH} EQUAL 0 OR ${CLANG_LOCATION_LENGTH} EQUAL 0)
        message(FATAL_ERROR "clang not find!!!")
    endif ()

    message("====== use clang to compile ======")


    math(EXPR CLANG_LOCATION_LENGTH "${CLANG_LOCATION_LENGTH}-1")
    math(EXPR CLANG_CPP_LOCATION_LENGTH "${CLANG_CPP_LOCATION_LENGTH}-1")

    string(SUBSTRING ${CLANG_LOCATION} 0 ${CLANG_LOCATION_LENGTH} CLANG_LOCATION)
    string(SUBSTRING ${CLANG_CPP_LOCATION} 0 ${CLANG_CPP_LOCATION_LENGTH} CLANG_CPP_LOCATION)

    message("CMAKE_C_COMPILER is ${CLANG_LOCATION}")
    message("CMAKE_CXX_COMPILER is ${CLANG_CPP_LOCATION}")

    set(CMAKE_CXX_COMPILER ${CLANG_CPP_LOCATION})
    set(CMAKE_C_COMPILER ${CLANG_LOCATION})
else ()
    message("====== use other compiler to compile")
    message("CMAKE_C_COMPILER is ${CMAKE_C_COMPILER}")
    message("CMAKE_CXX_COMPILER is ${CMAKE_CXX_COMPILER}")
endif ()

project(DeftRPC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


if (CMAKE_BUILD_TYPE)
    if (CMAKE_BUILD_TYPE STREQUAL " Debug")
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


if (ENABLE_TEST STREQUAL "True ")
    enable_testing()
endif ()

add_executable(DeftRPC main.cpp)


add_subdirectory(./src)
add_subdirectory(./example)
add_subdirectory(./third_part)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)