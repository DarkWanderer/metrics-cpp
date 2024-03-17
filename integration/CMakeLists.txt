add_executable(integration-test integration.cpp)
target_link_libraries(integration-test PUBLIC METRICS::lib)
