include(FetchContent)

cmake_policy(SET CMP0135 NEW)

# nlohmann-json
FetchContent_Declare(
    nlohmann-json
    GIT_REPOSITORY https://github.com/nlohmann/json/
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(nlohmann-json)

# stream-quantiles
FetchContent_Declare(
    quantiles
    GIT_REPOSITORY  https://github.com/Liam0205/stream-quantiles
    GIT_TAG         fe65a645e85d4f6f112dd232e527d52524024548
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)

FetchContent_GetProperties(quantiles)
if(NOT quantiles_POPULATED)
  FetchContent_Populate(quantiles)
endif()

add_library(quantiles INTERFACE)
target_include_directories(quantiles INTERFACE ${quantiles_SOURCE_DIR})

# asio
FetchContent_Declare(
    asio
    GIT_REPOSITORY  https://github.com/chriskohlhoff/asio
    GIT_TAG         asio-1-28-1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)

FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
  FetchContent_Populate(asio)
endif()

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
