cmake_minimum_required(VERSION 3.25)
project(DeftRPC)

aux_source_directory(. CLSN_HASH)

message("=============CLSN_HASH=============")
message(${CLSN_HASH})
add_library(CLSN_HASH SHARED ${CLSN_HASH})
target_include_directories(CLSN_HASH PUBLIC ${CMAKE_SOURCE_DIR}/include/)



