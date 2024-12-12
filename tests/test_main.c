#ifdef _WIN32
#include <direct.h>
#include <sysinfoapi.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "unity.h"
#include "test_shared.h"
#include "file_list.h"


void setUp(void) {
    // Create a temporary directory structure:
    // temp_dir/
    //  file1.txt (some content)
    //  subdir/
    //    nested.txt
    memset(&config, 0, sizeof(config));

    file_list_init(&list);

    memset(temp_dir, 0, sizeof(temp_dir));
#ifdef _WIN32
    sprintf(temp_dir, "test_temp_%u", (unsigned)GetTickCount());
    _mkdir(temp_dir);
    printf("[DEBUG] Created directory %s\n", temp_dir);
    char subdir[512];
    snprintf(subdir, sizeof(subdir), "%s/subdir", temp_dir);
    _mkdir(subdir);
    printf("[DEBUG] Created subdirectory: %s\n", subdir);
#else
    snprintf(temp_dir, sizeof(temp_dir), "test_temp_%u", (unsigned)rand());
    mkdir(temp_dir, 0700);
    printf("[DEBUG] Created directory: %s\n", temp_dir);
    char subdir[512];
    snprintf(subdir, sizeof(subdir), "%s/subdir", temp_dir);
    mkdir(subdir, 0700);
    printf("[DEBUG] Created subdirectory: %s\n", subdir);
#endif

    // Create files:
    char file1[512];
    snprintf(file1, sizeof(file1), "%s/file1.txt", temp_dir);
    FILE *fp = fopen(file1, "wb");
    printf("[DEBUG] Created file: %s\n", file1);
    TEST_ASSERT_NOT_NULL_MESSAGE(fp, "Failed to create test file.txt");
    fputs("Hello", fp);
    fclose(fp);

    // Create a file in the subdirectory
    char nested[512];
    snprintf(nested, sizeof(nested), "%s/subdir/nested.txt", temp_dir);
    fp = fopen(nested, "wb");
    printf("[DEBUG] Created file: %s\n", nested);
    TEST_ASSERT_NOT_NULL_MESSAGE(fp, "Failed to create nested.txt");
    fputs("World", fp);
    fclose(fp);
}

void tearDown(void) {
    // Cleanup temp_dir after tests - platform-specific removal
#ifdef _WIN32
    // On windos remove files first, then directories
    char file1[512], nested[512], subdir[512];
    snprintf(file1, sizeof(file1), "%s\\file1.txt", temp_dir);
    snprintf(nested, sizeof(nested), "%s\\subdir\\nested.txt", temp_dir);
    snprintf(subdir, sizeof(subdir), "%s\\subdir", temp_dir);

    remove(file1);
    remove(nested);
    _rmdir(subdir);
    _rmdir(temp_dir);
#else
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    int ret = system(cmd);

    // Check return value for system command
    if (ret != 0) {
        fprintf(stderr, "Warning: system command failed with return code %d\n", ret);
    }

#endif
file_list_free(&list);
}

// Forward declarations of test functions from test_platform.c
void test_directory_ops(void);
void test_directory_error(void);
void test_stat_file(void);
void test_fopen(void);
void test_unicode_paths(void);
void test_error_messages(void);

// Forward declarations of test functions from test_config.c
void test_parse_args_valid(void);
void test_parse_args_missing_input(void);
void test_parse_args_input_no_value(void);
void test_parse_args_missing_output(void);
void test_parse_args_output_no_value(void);
void test_parse_args_unknown_option(void);
void test_parse_args_help(void);
void test_parse_args_no_recursion(void);

// Forward declarations of test functions from test_file_list.c
void test_file_list_init(void);
void test_file_list_append_single(void);
void test_file_list_append_multiple(void);
void test_file_list_free(void);

// Forward declarations of test functions from test_scan.c
void test_scan_empty_dir(void);
void test_scan_single_file(void);
void test_scan_subdirs_no_recursion(void);
void test_scan_subdirs_with_recursion(void);


int main(void) {
    UNITY_BEGIN();

    // Run platform tests
    RUN_TEST(test_directory_ops);
    RUN_TEST(test_directory_error);
    RUN_TEST(test_stat_file);
    RUN_TEST(test_fopen);
#ifdef _WIN32
    RUN_TEST(test_unicode_paths);
#endif
    RUN_TEST(test_error_messages);

    // Run config/argument parsing tests
    RUN_TEST(test_parse_args_valid);
    RUN_TEST(test_parse_args_missing_input);
    RUN_TEST(test_parse_args_input_no_value);
    RUN_TEST(test_parse_args_missing_output);
    RUN_TEST(test_parse_args_output_no_value);
    RUN_TEST(test_parse_args_unknown_option);
    RUN_TEST(test_parse_args_help);
    RUN_TEST(test_parse_args_no_recursion);

    // Run file_list tests
    RUN_TEST(test_file_list_init);
    RUN_TEST(test_file_list_append_single);
    RUN_TEST(test_file_list_append_multiple);
    RUN_TEST(test_file_list_free);

    // Run scan tests
    RUN_TEST(test_scan_empty_dir);
    RUN_TEST(test_scan_single_file);
    RUN_TEST(test_scan_subdirs_no_recursion);
    RUN_TEST(test_scan_subdirs_with_recursion);

    return UNITY_END();
}
