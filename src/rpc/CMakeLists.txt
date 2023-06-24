cmake_minimum_required(VERSION 3.25)
project(DeftRPC)


aux_source_directory(. RPC_SRC)

message("=============RPC_SRC=============")
message(${RPC_SRC})

add_library(CLSN_RPC SHARED ${RPC_SRC})
target_include_directories(CLSN_RPC PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(CLSN_RPC LOG CLSN_DATASTRUCT)



