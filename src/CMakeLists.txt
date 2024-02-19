include(GenerateExportHeader)

if (MSVC)
    add_compile_options(/Wall /wd4625 /wd4626 /wd4514 /wd5045)
else()
    add_compile_options(-Wall -Wno-unknown-pragmas)
endif()

file(GLOB METRICS_SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(metrics SHARED ${METRICS_SOURCE_FILES})
target_include_directories(metrics PUBLIC "${PROJECT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(metrics PRIVATE nlohmann_json quantiles asio)
generate_export_header(metrics)
add_library(METRICS::lib ALIAS metrics)
