cmake_minimum_required(VERSION 3.19)
project(metrics-cpp VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CTest)
enable_testing()

add_subdirectory(src)
add_subdirectory(test)
