message(========third_part========)
if (ENABLE_TEST STREQUAL "True")
    add_subdirectory(./googletest)
endif ()

add_subdirectory(./highwayhash)