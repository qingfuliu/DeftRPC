cmake_minimum_required(VERSION 3.22)
project(CLSN_LOG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} test/LogConvertTest.cpp)

include_directories(${PROJECT_NAME}  PRIVATE ./)