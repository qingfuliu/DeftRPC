cmake_minimum_required(VERSION 3.25)
project(DeftRPC)


aux_source_directory(. NET_SRC)
message("=============NET_SRC=============")
message(${NET_SRC})
add_library(CLSN_NET SHARED ${NET_SRC})
target_include_directories(CLSN_NET PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(CLSN_NET CLSN_HOOK LOG CLSN_COROUTINE pthread)


aux_source_directory(./sever SEVER_SRC)
message("=============SEVER_SRC=============")
message(${SEVER_SRC})
add_library(CLSN_SEVER SHARED ${SEVER_SRC})
target_include_directories(CLSN_SEVER PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(CLSN_SEVER CLSN_HOOK CLSN_NET LOG CLSN_COROUTINE pthread)

aux_source_directory(./client CLIENT_SRC)
message("=============CLIENT_SRC=============")
message(${CLIENT_SRC})
add_library(CLSN_CLIENT SHARED ${CLIENT_SRC})
target_include_directories(CLSN_CLIENT PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(CLSN_CLIENT LOG CLSN_NET CLSN_COROUTINE pthread CLSN_HOOK)
