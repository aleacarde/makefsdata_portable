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

# Enable AddressSanitizer if requested
if(ENABLE_ASAN)
    message(STATUS "Enabling AddressSanitizer for tests")
    target_compile_options(test_platform PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(test_platform PRIVATE -fsanitize=address)
endif()

add_test(NAME platform_tests COMMAND test_platform)

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
        platform_tests PROPERTIES ENVIRONMENT "LD_PRELOAD=${ASAN_ENV}"
    )

endif()