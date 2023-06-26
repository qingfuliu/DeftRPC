project(DeftRPC)

message(========build_googletest========)
if (ENABLE_TEST STREQUAL "True")
    add_subdirectory(./googletest)
endif ()