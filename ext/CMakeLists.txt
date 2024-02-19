include(FetchContent)

cmake_policy(SET CMP0135 NEW)

if (MSVC)
    add_compile_options(/w)
else()
    add_compile_options(-w)
endif()

find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
