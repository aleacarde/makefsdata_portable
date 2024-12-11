#include "unity.h"
#include "config.h"
#include "test_shared.h"
#include <string.h>
#include <stdbool.h>

// Test: Valid arguments with all mandatory options
void test_parse_args_valid(void) {
    char *argv[] = {
        "makefsdata_portable",
        "--input", "webfiles",
        "--output", "fsdata.c",
        "--recursive"
    };
    int argc = (int)(sizeof(argv) / sizeof(argv[0]));

    bool result = parse_args(argc, argv, &config);
    TEST_ASSERT_TRUE_MESSAGE(result, "Expected parse_args to succeed with valid arguments");

    TEST_ASSERT_EQUAL_STRING("webfiles", config.input_dir);
    TEST_ASSERT_EQUAL_STRING("fsdata.c", config.output_file);
    TEST_ASSERT_TRUE(config.recursive);
    TEST_ASSERT_FALSE(config.show_help);
}

// Test: Missing --input argument
void test_parse_args_missing_input(void) {
    char *argv[] = {
        "makefsdata_portable",
        "--output", "fsdata.c"
    };
    int argc = (int)(sizeof(argv) / sizeof(argv[0]));

    bool result = parse_args(argc, argv, &config);
    TEST_ASSERT_FALSE_MESSAGE(result, "Expected parse_args to fail when --input is missing");
}

// Test: Missing value after --input
void test_parse_args_input_no_value(void) {
    char *argv[] = {
        "makefsdata_portable",
        "--input",
        "--output", "fsdata.c"
    };
    int argc = (int)(sizeof(argv) / sizeof(argv[0]));

    bool result = parse_args(argc, argv, &config);
    TEST_ASSERT_FALSE_MESSAGE(result, "Expected parse_args to fail when no value is provided for --input");
}

// Test: Mising --output argument
void test_parse_args_missing_output(void) {
    char *argv[] = {
        "makefsdata_portable",
        "--input", "webfiles"
    };
    int argc = (int)(sizeof(argv) / sizeof(argv[0]));

    bool result = parse_args(argc, argv, &config);
    TEST_ASSERT_FALSE_MESSAGE(result, "Expected parse_args to fail when --output is missing");
}

// Test: Missing value after --output
void test_parse_args_output_no_value(void) {
    char *argv[] = {
        "makefsdata_portable",
        "--input", "webfiles",
        "--output"
    };
    int argc = (int)(sizeof(argv) / sizeof(argv[0]));

    bool result = parse_args(argc, argv, &config);
    TEST_ASSERT_FALSE_MESSAGE(result, "Expected parse_args to fail when no value is provided for --output");
}

// Test: Unknown option
void test_parse_args_unknown_option(void) {
    char *argv[] = {
        "makefsdata_portable",
        "--input", "webfiles",
        "--unknown", "something",
        "--output", "fsdata.c"
    };
    int argc = (int)(sizeof(argv) / sizeof(argv[0]));

    bool result = parse_args(argc, argv, &config);
    TEST_ASSERT_FALSE_MESSAGE(result, "Expected parse_args to fail on unknown option");
}

// Test: Help option
void test_parse_args_help(void) {
    char *argv[] = {
        "makefsdata_portable",
        "--help"
    };
    int argc = (int)(sizeof(argv) / sizeof(argv[0]));

    bool result = parse_args(argc, argv, &config);
    TEST_ASSERT_TRUE_MESSAGE(result, "Expected parse_args to succeed with --help");
    TEST_ASSERT_TRUE_MESSAGE(config.show_help, "Expected show_help to be true when --help is given");
}

// Test: Valid arguments without recursion
void test_parse_args_no_recursion(void) {
    char *argv[] = {
        "makefsdata_portable",
        "--input", "webfiles",
        "--output", "fsdata.c"
    };
    int argc = (int)(sizeof(argv) / sizeof(argv[0]));

    bool result = parse_args(argc, argv, &config);
    TEST_ASSERT_TRUE_MESSAGE(result, "Expected parse_args to succeed");
    TEST_ASSERT_EQUAL_STRING("webfiles", config.input_dir);
    TEST_ASSERT_EQUAL_STRING("fsdata.c", config.output_file);
    TEST_ASSERT_FALSE(config.recursive);
    TEST_ASSERT_FALSE(config.show_help);
}
