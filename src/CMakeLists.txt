include(GenerateExportHeader)

if (MSVC)
    add_compile_options(/Wall /wd4625 /wd4626 /wd4514)
else()
    add_compile_options(-Wall -Wno-unknown-pragmas)
endif()

add_library(metrics SHARED 
    json.cpp
    metrics.cpp
    prometheus.cpp
    registry.cpp
    statsd.cpp
)
generate_export_header(metrics)

target_include_directories(metrics PUBLIC "${PROJECT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}")

add_library(METRICS::lib ALIAS metrics)

target_link_libraries(metrics PRIVATE nlohmann_json::nlohmann_json quantiles)
