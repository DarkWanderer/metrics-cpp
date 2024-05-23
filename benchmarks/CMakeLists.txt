include(FetchContent)

message(STATUS "Cloning Google Benchmark")
set(BENCHMARK_ENABLE_TESTING NO)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.4
)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(metrics_benchmark benchmarks.cpp)
target_link_libraries(metrics_benchmark PUBLIC METRICS::lib benchmark::benchmark)
