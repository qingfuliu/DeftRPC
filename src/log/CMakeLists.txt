aux_source_directory(${CMAKE_SOURCE_DIR}/src/log LOG_SRC)

add_library(LOG SHARED ${LOG_SRC})

target_link_libraries(LOG -pthread)
