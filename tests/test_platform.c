#include "unity.h"
#include "platform.h"
#include "test_shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Test platform_opendir and related functions
void test_directory_ops(void) {
    platform_dir_handle *dh = platform_opendir(temp_dir);
    TEST_ASSERT_NOT_NULL_MESSAGE(dh, platform_get_last_error());

    printf("[DEBUG] Opened directory: %s\n", temp_dir);

    platform_file_info info;
    int rc;

    // We know temp_dir has 'file1.txt' and 'subdir'
    int file_count = 0;
    int dir_count = 0;

    while ((rc = platform_readdir(dh, &info)) == 0) {
        printf("[DEBUG] Found entry: %s at %p\n", info.name, (void *)info.name);
    
        TEST_ASSERT_NOT_NULL_MESSAGE(info.name, "info.name should not be NULL");
        if (info.is_dir) {
            printf("[DEBUG] Entry is a directory: %s\n", info.name);
            dir_count++;
        } else {
            printf("[DEBUG] Entry is a file: %s (size: %llu bytes)\n", info.name, (unsigned long long)info.size);
            file_count++;
        }
    }

    TEST_ASSERT_MESSAGE(rc == -1, "readdir should return -1 at the end");

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, file_count, "Expected 1 file in dir");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, dir_count, "Expected 1 subdir");

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, platform_closedir(dh), platform_get_last_error());
    printf("[DEBUG] Closed directory: %s\n", temp_dir);
}

void test_directory_error(void) {
    // Nonexistent directory
    platform_dir_handle *dh = platform_opendir("no_such_dir_12345");
    TEST_ASSERT_NULL_MESSAGE(dh, "Expected NULL for nonexistent directory");
    TEST_ASSERT_NOT_EQUAL_MESSAGE('\0', platform_get_last_error()[0], "Expected error message");
}

void test_stat_file(void) {
    char file1[512];
    snprintf(file1, sizeof(file1), "%s/file1.txt", temp_dir);
    platform_file_info info;

    int rc = platform_stat_file(file1, &info);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, rc, platform_get_last_error());
    TEST_ASSERT_EQUAL_INT(0, info.is_dir);
    TEST_ASSERT_EQUAL_INT(5, (int)info.size); // "Hello" is 5 bytes

    // Stat subdir
    char subd[512];
    snprintf(subd, sizeof(subd), "%s/subdir", temp_dir);
    rc = platform_stat_file(subd, &info);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, rc, platform_get_last_error());
    TEST_ASSERT_EQUAL_INT(1, info.is_dir);
    TEST_ASSERT_EQUAL_INT(0, (int)info.size);

    // Nonexistent file
    rc = platform_stat_file("no_such_file_abc", &info);
    TEST_ASSERT_NOT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_EQUAL_MESSAGE('\0', platform_get_last_error()[0], "Expected error message");
}

void test_fopen(void) {
    char file1[512];
    snprintf(file1, sizeof(file1), "%s/file1.txt", temp_dir);

    FILE *fp = platform_fopen(file1);
    TEST_ASSERT_NOT_NULL_MESSAGE(fp, platform_get_last_error());
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    TEST_ASSERT_EQUAL_INT(5, (int)size);
    fclose(fp);

    // Nonexistent file
    fp = platform_fopen("no_such_file_xyz");
    TEST_ASSERT_NULL(fp);
    TEST_ASSERT_NOT_EQUAL_MESSAGE('\0', platform_get_last_error()[0], "Expected error message");
}

#ifdef _WIN32
void test_unicode_paths(void) {
    // Test a filename with unicode chars
    // We'll create a file with a unicode name.
    // For simplicity, we pick a known safe unichode char e.g. U+00E9 (é)

    char utf8name[512];
    snprintf(utf8name, sizeof(utf8name), "%s/subdir/\xC3\xA9test.txt", temp_dir); // "étest.txt"

    FILE *fp = fopen(utf8name, "wb");
    TEST_ASSERT_NOT_NULL_MESSAGE(fp, "Failed to create unicode test file");
    fputs("Data", fp);
    fclose(fp);

    // Now open with platform_fopen
    FILE *fp2 = platform_fopen(utf8name);
    TEST_ASSERT_NOT_NULL_MESSAGE(fp2, platform_get_last_error());
    fseek(fp2, 0, SEEK_END);
    long sz = ftell(fp2);
    TEST_ASSERT_EQUAL_INT(4, (int)sz);
    fclose(fp2);
}
#endif

// Test error setting
void test_error_messages(void) {
    // Call something with invalid args
    int rc = platform_stat_file(NULL, NULL);
    TEST_ASSERT_NOT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_EQUAL('\0', platform_get_last_error()[0]);
}

