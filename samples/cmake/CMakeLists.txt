cmake_minimum_required(VERSION 3.24)
project(metric-cpp-sample)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)

FetchContent_Declare(
    metrics
    GIT_REPOSITORY https://github.com/DarkWanderer/metrics-cpp
    GIT_TAG        main
)
FetchContent_MakeAvailable(metrics)

add_executable(sample sample.cpp)
target_link_libraries(sample PRIVATE METRICS::lib)
