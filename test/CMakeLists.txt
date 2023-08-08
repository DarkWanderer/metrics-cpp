Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.2.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests tests.cpp)
add_dependencies(tests metrics)
target_link_libraries(tests PUBLIC metrics PRIVATE Catch2::Catch2WithMain)
add_custom_command(TARGET tests POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:metrics>" "$<TARGET_FILE_DIR:tests>")

include(Catch)
catch_discover_tests(tests)  # does not work?
#add_test(NAME tests COMMAND $<TARGET_FILE:tests>)
