Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.2.1
)
FetchContent_MakeAvailable(Catch2)

file(GLOB TESTS_SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(tests ${TESTS_SOURCE_FILES})
target_link_libraries(tests PUBLIC METRICS::lib PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(tests)
