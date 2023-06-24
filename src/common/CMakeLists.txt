cmake_minimum_required(VERSION 3.25)
project(DeftRPC)


aux_source_directory(. COMMON_SRC)

message("=============COMMON_SRC=============")
message(${COMMON_SRC})

add_library(CLSN_COMMON SHARED ${COMMON_SRC})
target_include_directories(CLSN_COMMON PUBLIC ${CMAKE_SOURCE_DIR}/include/)



