include(GenerateExportHeader)

if (MSVC)
    add_compile_options(/Wall /wd4625 /wd4626 /wd4514 /wd5045)
else()
    add_compile_options(-Wall -Wno-unknown-pragmas)
endif()

file(GLOB_RECURSE METRICS_SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE METRICS_HEADER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../include/*.h")

add_library(metrics SHARED ${METRICS_SOURCE_FILES})
target_include_directories(metrics PUBLIC "${PROJECT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(metrics PRIVATE Boost::asio Boost::accumulators Boost::beast Boost::json Boost::url)
generate_export_header(metrics)
add_library(METRICS::lib ALIAS metrics)

if (METRICS_BUILD_SANITIZER)
  add_compile_options(metrics PUBLIC /fsanitize=${METRICS_BUILD_SANITIZER} /Zi /Oy-)
endif()

# Data to properly create install package
set_target_properties(metrics PROPERTIES PUBLIC_HEADER "${METRICS_HEADER_FILES}")
install(TARGETS metrics PUBLIC_HEADER)
