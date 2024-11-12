aux_source_directory(. HOOK_SRC)

add_library(CLSN_HOOK SHARED ${HOOK_SRC})
target_link_libraries(CLSN_HOOK LOG CLSN_COROUTINE dl)



