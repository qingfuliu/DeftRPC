cmake_minimum_required(VERSION 3.22)
project(DeftRPC)

aux_source_directory(${CMAKE_SOURCE_DIR}/src/log LOG_SRC)

add_library(LOG SHARED ${LOG_SRC})

target_include_directories(LOG PUBLIC ${CMAKE_SOURCE_DIR}/include/log)
target_include_directories(LOG PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(LOG -pthread)
