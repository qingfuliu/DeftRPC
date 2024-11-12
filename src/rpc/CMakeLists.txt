aux_source_directory(. RPC_SRC)



add_library(CLSN_RPC SHARED ${RPC_SRC})
target_link_libraries(CLSN_RPC LOG CLSN_DATASTRUCT CLSN_COMMON CLSN_NET)


aux_source_directory(./sever RPC_SEVER_SRC)

add_library(CLSN_RPC_SEVER SHARED ${RPC_SEVER_SRC})
target_link_libraries(CLSN_RPC_SEVER CLSN_RPC CLSN_SEVER)

aux_source_directory(./client RPC_CLIENT_SRC)

add_library(CLSN_RPC_CLIENT SHARED ${RPC_CLIENT_SRC})
target_link_libraries(CLSN_RPC_CLIENT CLSN_RPC CLSN_CLIENT)

