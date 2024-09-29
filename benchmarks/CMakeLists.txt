set(BENCHMARK_ENABLE_TESTING NO)
find_package(benchmark CONFIG REQUIRED)
add_executable(metrics_benchmark benchmarks.cpp)
target_link_libraries(metrics_benchmark PUBLIC METRICS::lib benchmark::benchmark)
