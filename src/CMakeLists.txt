include(GenerateExportHeader)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall)
endif()

add_library(metrics SHARED 
    metrics.cpp
    registry.cpp
)
generate_export_header(metrics)

target_include_directories(metrics PUBLIC "${PROJECT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}")
