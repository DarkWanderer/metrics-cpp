add_executable(benchmarks benchmarks.cpp)
target_link_libraries(benchmarks PUBLIC METRICS::lib benchmark::benchmark)
