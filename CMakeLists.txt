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

# Boost
set(BOOST_INCLUDE_LIBRARIES asio accumulators beast json url)
set(BOOST_USE_STATIC_LIBS ON)
set(BOOST_ENABLE_CMAKE ON)
if (METRICS_BUILD_SANITIZER)
  set(BOOST_USE_ASAN ON)
endif()

find_package(Boost COMPONENTS asio accumulators beast json url REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)
find_package(curl CONFIG REQUIRED)

message(STATUS "Boost_INCLUDE_DIRS: [${Boost_INCLUDE_DIRS}]")
message(STATUS "Boost_LIBRARIES: [${Boost_LIBRARIES}]")

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

add_subdirectory(src)

if (METRICS_BUILD_TESTS OR METRICS_BUILD_BENCHMARKS)
    include(CTest)
    enable_testing()
endif()

if (METRICS_BUILD_TESTS)
    add_subdirectory(integration)
    add_subdirectory(test)
endif()

if (METRICS_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
