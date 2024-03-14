include(FetchContent)

cmake_policy(SET CMP0135 NEW)

if (MSVC)
    add_compile_options(/w)
else()
    add_compile_options(-w -fPIC)
endif()

# stream-quantiles
FetchContent_Declare(
    quantiles
    GIT_REPOSITORY  https://github.com/Liam0205/stream-quantiles
    GIT_TAG         fe65a645e85d4f6f112dd232e527d52524024548
    GIT_SHALLOW     TRUE
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)

FetchContent_MakeAvailable(quantiles)

add_library(quantiles INTERFACE)
target_include_directories(quantiles INTERFACE ${quantiles_SOURCE_DIR})

# Boost
set(BOOST_INCLUDE_LIBRARIES asio url json)
set(BOOST_USE_STATIC_LIBS ON)
set(BOOST_ENABLE_CMAKE ON)
message(STATUS "Downloading and extracting boost library sources. This will take some time...")
set(FETCHCONTENT_QUIET FALSE) # Needed to print downloading progress
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z # downloading a zip release speeds up the download
    USES_TERMINAL_DOWNLOAD TRUE
    GIT_PROGRESS TRUE
    DOWNLOAD_NO_EXTRACT FALSE
)
FetchContent_MakeAvailable(Boost)
