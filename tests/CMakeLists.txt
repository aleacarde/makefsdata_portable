set(TEST_SOURCES
    test_main.c
    test_shared.c
    test_platform.c
    test_config.c
    test_file_list.c
    test_scan.c
    unity.c
)

add_executable(run_tests ${TEST_SOURCES})

target_include_directories(run_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src  # For source-specific headers
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests  # For test-specific headers
)
target_link_libraries(run_tests PRIVATE makefsdata_portable) # linking the library that includes platform.c

set_target_properties(run_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)

set(TEST_RESOURCES_DIR ${CMAKE_SOURCE_DIR}/tests/resources)
target_compile_definitions(run_tests PRIVATE TEST_RESOURCES_DIR=\"${TEST_RESOURCES_DIR}\")

# Enable AddressSanitizer if requested
if(ENABLE_ASAN)
    message(STATUS "Enabling AddressSanitizer for tests")
    target_compile_options(run_tests PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(run_tests PRIVATE -fsanitize=address)
endif()

add_test(NAME tests_run COMMAND ${CMAKE_BINARY_DIR}/bin/tests/run_tests)

if(ENABLE_ASAN AND UNIX)
    find_program(GCC_PATH gcc)
    execute_process(
        COMMAND ${GCC_PATH} -print-file-name=libasan.so
        OUTPUT_VARIABLE ASAN_LIBRARY
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(ASAN_ENV "LD_PRELOAD=${ASAN_LIBRARY}")
endif()

if(ENABLE_ASAN AND ASAN_ENV)
    set_tests_properties(
        tests_run PROPERTIES ENVIRONMENT "LD_PRELOAD=${ASAN_ENV}"
    )

endif()