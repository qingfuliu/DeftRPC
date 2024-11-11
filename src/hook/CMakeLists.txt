cmake_minimum_required(VERSION 3.25)
project(DeftRPC)


aux_source_directory(. HOOK_SRC)

add_library(CLSN_HOOK SHARED ${HOOK_SRC})
target_include_directories(CLSN_HOOK PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(CLSN_HOOK LOG CLSN_COROUTINE dl)



