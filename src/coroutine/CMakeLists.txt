cmake_minimum_required(VERSION 3.25)
project(DeftRPC)


set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -g -fno-strict-aliasing -O2 -Wall -export-dynamic -Wall -pipe -D_GNU_SOURCE -D_REENTRANT -fPIC -Wno-deprecated -m64)
# Use c and asm
enable_language(C ASM)

aux_source_directory(. COROUTINE_SRC)

list(APPEND COROUTINE_SRC ./coctx_swap.S)

message("=============COROUTINE_SRC=============")
message(${COROUTINE_SRC})
add_library(CLSN_COROUTINE SHARED ${COROUTINE_SRC})
target_include_directories(CLSN_COROUTINE PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(CLSN_COROUTINE LOG)



