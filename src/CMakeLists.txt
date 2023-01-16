include(GenerateExportHeader)

add_library(metrics SHARED 
    gauge.cpp 
    counter.cpp 
    histogram.cpp
    registry.cpp
)
generate_export_header(metrics)

target_include_directories(metrics PUBLIC "${PROJECT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}")
