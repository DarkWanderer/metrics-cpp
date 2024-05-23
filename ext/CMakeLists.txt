include(FetchContent)

cmake_policy(SET CMP0135 NEW)

if (MSVC)
    add_compile_options(/w)
else()
    add_compile_options(-w -fPIC)
endif()

# Boost
set(BOOST_INCLUDE_LIBRARIES asio accumulators beast json url)
set(BOOST_USE_STATIC_LIBS ON)
set(BOOST_ENABLE_CMAKE ON)
if (METRICS_BUILD_SANITIZER)
  set(BOOST_USE_ASAN ON)
endif()

message(STATUS "Downloading and extracting boost library sources. This will take some time...")
set(FETCHCONTENT_QUIET FALSE) # Needed to print downloading progress
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z # downloading a zip release speeds up the download
    USES_TERMINAL_DOWNLOAD TRUE
    GIT_PROGRESS TRUE
    DOWNLOAD_NO_EXTRACT FALSE
)
FetchContent_MakeAvailable(Boost)

message(STATUS "Cloning Google Benchmark")
set(BENCHMARK_ENABLE_TESTING NO)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.4
)
FetchContent_MakeAvailable(googlebenchmark)
