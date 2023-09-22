cmake_minimum_required(VERSION 3.19)
project(metrics-cpp VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(SANITIZE "Enable selected sanitizer in build" OFF)

include(sanitizer.cmake)
if(SANITIZE)
  enable_sanitizer(${SANITIZE})
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

add_subdirectory(ext)
add_subdirectory(src)

include(CTest)
enable_testing()
add_subdirectory(test)
