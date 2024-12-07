add_executable(test_platform test_main.c test_platform.c unity.c)

target_include_directories(test_platform PRIVATE
    ${CMAKE_SOURCE_DIR}/src  # For source-specific headers
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests  # For test-specific headers
)
target_link_libraries(test_platform PRIVATE makefsdata_portable) # linking the library that includes platform.c

set_target_properties(test_platform PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)

add_test(NAME platform_tests COMMAND test_platform)