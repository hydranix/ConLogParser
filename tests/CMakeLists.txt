add_executable(test_parsers_unit test_parsers_unit.cpp)
target_link_libraries(test_parsers_unit
    PRIVATE
        conlog_core
        $<BUILD_INTERFACE:conlog_warnings>
)
target_include_directories(test_parsers_unit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_parsers_e2e test_parsers_e2e.cpp)
target_link_libraries(test_parsers_e2e
    PRIVATE
        conlog_core
        $<BUILD_INTERFACE:conlog_warnings>
)
target_include_directories(test_parsers_e2e PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(test_parsers_e2e PRIVATE
    CONLOG_TEST_SAMPLE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/conlog_sample.txt"
)

add_test(NAME parsers_unit COMMAND test_parsers_unit)
add_test(NAME parsers_e2e  COMMAND test_parsers_e2e)
