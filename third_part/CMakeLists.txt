message(========third_part========)
if (ENABLE_TEST STREQUAL "True")
    add_subdirectory(./googletest)

    add_subdirectory(./benchmark)
endif ()

add_subdirectory(./highwayhash)