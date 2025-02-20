#add_subdirectory(./common)
#add_subdirectory(./log)
#add_subdirectory(./coroutine)
#add_subdirectory(./net)
#add_subdirectory(./hook)
#add_subdirectory(./dataStruct)

# Use c and asm
enable_language(C ASM)

file(GLOB_RECURSE TEST_SOURCE LIST_DIRECTORIES false ./*.cpp ./*.S)

add_library(DeftRpc STATIC ${TEST_SOURCE})

target_include_directories(DeftRpc PRIVATE ${CMAKE_SOURCE_DIR}/src/include PUBLIC ${CMAKE_SOURCE_DIR}/include/log)

target_link_libraries(DeftRpc PUBLIC pthread dl highwayhash)


