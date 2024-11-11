cmake_minimum_required(VERSION 3.25)
project(DeftRPC)

aux_source_directory(. DATASTRUCT_SRC)

add_library(CLSN_DATASTRUCT SHARED ${DATASTRUCT_SRC})
target_include_directories(CLSN_DATASTRUCT PUBLIC ${CMAKE_SOURCE_DIR}/include/)



