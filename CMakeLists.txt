cmake_minimum_required(VERSION 3.19)
project(metrics-cpp VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(METRICS_BUILD_SANITIZER "Enable selected sanitizer in build (address/thread/leak/undefined)" OFF)
option(METRICS_BUILD_TESTS "Build tests for this library" OFF)

include(sanitizer.cmake)
if(METRICS_BUILD_SANITIZER)
  enable_sanitizer(${METRICS_BUILD_SANITIZER})
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

add_subdirectory(ext)
add_subdirectory(src)

if (METRICS_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(benchmarks)
    add_subdirectory(integration)
    add_subdirectory(test)
endif()
