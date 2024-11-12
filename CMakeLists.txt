cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 17) # Compile as C++17.
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Require C++17 support.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clang-tidy.
include(GNUInstallDirs)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

project(DeftRPC
        VERSION 1.0
        DESCRIPTION "An automatic clicker based on small target recognition"
        LANGUAGES C CXX)

# People keep running CMake in the wrong folder, completely nuking their project or creating weird bugs.
# This checks if you're running CMake from a folder that already has CMakeLists.txt.
# Importantly, this catches the common case of running it from the root directory.
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" PATH_TO_CMAKELISTS_TXT)

if(EXISTS "${PATH_TO_CMAKELISTS_TXT}")
    message(FATAL_ERROR "Run CMake from a build subdirectory! \"mkdir build ; cd build ; cmake ..\" \
    Some junk files were created in this folder (CMakeCache.txt, CMakeFiles); you should delete those.")
endif()


# For compiler type.
if (USE_CLANG STREQUAL "True")
    execute_process(COMMAND which clang OUTPUT_VARIABLE CLANG_LOCATION)
    string(LENGTH ${CLANG_LOCATION} CLANG_LOCATION_LENGTH)

    execute_process(COMMAND which clang++ OUTPUT_VARIABLE CLANG_CPP_LOCATION)
    string(LENGTH ${CLANG_CPP_LOCATION} CLANG_CPP_LOCATION_LENGTH)

    if (${CLANG_CPP_LOCATION_LENGTH} EQUAL 0 OR ${CLANG_LOCATION_LENGTH} EQUAL 0)
        message(FATAL_ERROR "clang not find!!!")
    endif ()

    message("====== use clang to compile ======")


    math(EXPR CLANG_LOCATION_LENGTH "${CLANG_LOCATION_LENGTH}-1")
    math(EXPR CLANG_CPP_LOCATION_LENGTH "${CLANG_CPP_LOCATION_LENGTH}-1")

    string(SUBSTRING ${CLANG_LOCATION} 0 ${CLANG_LOCATION_LENGTH} CLANG_LOCATION)
    string(SUBSTRING ${CLANG_CPP_LOCATION} 0 ${CLANG_CPP_LOCATION_LENGTH} CLANG_CPP_LOCATION)

    message("CMAKE_C_COMPILER is ${CLANG_LOCATION}")
    message("CMAKE_CXX_COMPILER is ${CLANG_CPP_LOCATION}")

    set(CMAKE_CXX_COMPILER ${CLANG_CPP_LOCATION})
    set(CMAKE_C_COMPILER ${CLANG_LOCATION})
else ()
    message("====== use other compiler to compile")
    message("CMAKE_C_COMPILER is ${CMAKE_C_COMPILER}")
    message("CMAKE_CXX_COMPILER is ${CMAKE_CXX_COMPILER}")
endif ()

# For compile type.
if (CMAKE_BUILD_TYPE)
    if (CMAKE_BUILD_TYPE STREQUAL " Debug")
        message("======Debug======")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -O0")
        add_compile_definitions(DEBUG)
    else ()
        message("======Release======")
        add_compile_definitions(RELEASE)
    endif ()
else ()
    message("======Debug======")
    message(STATUS "Setting build type to `Debug` as none was specified.")
    add_compile_definitions(DEBUG)
endif ()

# For test.
if ((NOT DEFINED ENABLE_TEST) OR ("${ENABLE_TEST}"))
    set(ENABLE_TEST "True")
    enable_testing()
endif ()

# #####################################################################################################################
# FORMAT CHECK TOOLS
# #####################################################################################################################

# For format check tools
set(FORMAT_CHECK_TOOLS_SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/scripts/tools")
set(FORMAT_CHECK_TOOLS_SEARCH_PATH "/usr/local/bin" "/usr/bin" "/usr/local/opt/llvm/bin")
file(GLOB_RECURSE LINT_FILES
        "src/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/example/*.cpp"
)
file(GLOB_RECURSE TIDY_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/example/*.cpp"
)
# ###################For clang-format###################
# Attempt to find the binary clang-format.
# Users can use variable 「FORMAT_CHECK_TOOLS_SEARCH_PATH」 to specify the search path.
find_program(CLANG_FORMAT_BIN
        NAMES clang-format
        HINTS ${FORMAT_CHECK_TOOLS_SEARCH_PATH})
if ("${CLANG_FORMAT_BIN}" STREQUAL "CLANG_FORMAT_BIN-NOTFOUND")
    message(WARNING "Couldn't find clang-format.")
else ()
    message(STATUS "Found clang-format at ${CLANG_FORMAT_BIN}")
endif ()
# "make format"
# "make check-format"
string(CONCAT FORMAT_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/src,"
        "${CMAKE_CURRENT_SOURCE_DIR}/example,"
)
# Runs clang format and updates files in place.
add_custom_target(format ${FORMAT_CHECK_TOOLS_SCRIPTS_DIR}/run_clang_format.py
        ${CLANG_FORMAT_BIN}
        ${FORMAT_CHECK_TOOLS_SCRIPTS_DIR}/clang_format_exclusions.txt
        --source_dirs
        ${FORMAT_DIRS}
        --fix
        --quiet
)
# Runs clang format and exits with a non-zero exit code if any files need to be reformatted.
add_custom_target(check-format ${FORMAT_CHECK_TOOLS_SCRIPTS_DIR}/run_clang_format.py
        ${CLANG_FORMAT_BIN}
        ${FORMAT_CHECK_TOOLS_SCRIPTS_DIR}/clang_format_exclusions.txt
        --source_dirs
        ${FORMAT_DIRS}
        --quiet
)

# ###################For clang-tidy###################
# Attempt to find the binary clang-tidy.
# Users can use variable 「CLANG_TIDY_SEARCH_PATH」 to specify the search path.
find_program(CLANG_TIDY_BIN
        NAMES clang-tidy
        HINTS ${FORMAT_CHECK_TOOLS_SEARCH_PATH})
if ("${CLANG_TIDY_BIN}" STREQUAL "CLANG_TIDY_BIN-NOTFOUND")
    message(WARNING "Couldn't find clang-tidy.")
else ()
    # Output compile_commands.json.
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    message(STATUS "Found clang-tidy at ${CLANG_TIDY_BIN}")
endif ()

# For ###################clang-apply-replacements###################
find_program(CLANG_APPLY_REPLACEMENTS_BIN
        NAMES clang-apply-replacements
        HINTS ${FORMAT_CHECK_TOOLS_SEARCH_PATH})
if("${CLANG_APPLY_REPLACEMENTS_BIN}" STREQUAL "CLANG_APPLY_REPLACEMENTS_BIN-NOTFOUND")
    message(WARNING "Couldn't find clang-apply-replacements.")
else()
    # Output compile_commands.json.
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    message(STATUS "Found clang-apply-replacements at ${CLANG_APPLY_REPLACEMENTS_BIN}")
endif()

# "make check-clang-tidy"
add_custom_target(check-clang-tidy
        ${FORMAT_CHECK_TOOLS_SCRIPTS_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
)
add_custom_target(fix-clang-tidy
        ${FORMAT_CHECK_TOOLS_SCRIPTS_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
        -clang-apply-replacements-binary ${CLANG_APPLY_REPLACEMENTS_BIN} # using our clang-apply-replacements binary
        -fix # apply suggested changes generated by clang-tidy
)
add_custom_target(check-clang-tidy-diff
        ${FORMAT_CHECK_TOOLS_SCRIPTS_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
        -only-diff # only check diff files to master
        files ${LINT_FILES}
)
add_custom_target(fix-clang-tidy-diff
        ${FORMAT_CHECK_TOOLS_SCRIPTS_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
        -clang-apply-replacements-binary ${CLANG_APPLY_REPLACEMENTS_BIN} # using our clang-apply-replacements binary
        -fix # apply suggested changes generated by clang-tidy
        -only-diff # only check diff files to master
        files ${LINT_FILES}
)

# ###################For cpplint.py###################
find_program(CPPLINT_BIN
        NAMES cpplint cpplint.py
        HINTS "${FORMAT_CHECK_TOOLS_SCRIPTS_DIR}")

if("${CPPLINT_BIN}" STREQUAL "CPPLINT_BIN-NOTFOUND")
    message(WARNING "Couldn't find cpplint.")
else()
    message(STATUS "Found cpplint at ${CPPLINT_BIN}")
endif()
# "make check-lint"
add_custom_target(check-lint echo '${LINT_FILES}' | xargs -n12 -P8
        ${CPPLINT_BIN}
        --verbose=2 --quiet
        --linelength=120
        --filter=-legal/copyright,-build/header_guard,-runtime/references # https://github.com/cpplint/cpplint/issues/148
)

include_directories(${CMAKE_SOURCE_DIR}/src/include)
add_executable(DeftRPC main.cpp)
add_subdirectory(./src)
add_subdirectory(./example)
add_subdirectory(./third_part)

