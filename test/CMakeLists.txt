add_executable(
    tests
    tests.cpp
)
target_link_libraries(tests PUBLIC metrics)
add_custom_command(
    TARGET tests 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:metrics>" "$<TARGET_FILE_DIR:tests>")
add_test(NAME tests COMMAND $<TARGET_FILE:tests>)
