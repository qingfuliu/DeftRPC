aux_source_directory(. DATASTRUCT_SRC)
add_library(CLSN_DATASTRUCT SHARED ${DATASTRUCT_SRC})
target_link_libraries(CLSN_DATASTRUCT highwayhash)



