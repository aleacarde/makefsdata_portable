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

// Test opening an existing file in read mode
void test_platform_fopen_read(void) {
    platform_file_handle fh = platform_fopen(test_filename, "rb");
    TEST_ASSERT_NOT_NULL_MESSAGE(fh, "Failed to open existing file in rb mode.");
    platform_fclose(fh);
}

// Test reading from file
void test_platform_fread(void) {
    platform_file_handle fh = platform_fopen(test_filename, "rb");
    TEST_ASSERT_NOT_NULL_MESSAGE(fh, "Failed to open file for reading.");

    char buffer[64];
    size_t bytes_read = platform_fread(buffer, 1, sizeof(buffer)-1, fh);
    TEST_ASSERT_MESSAGE(bytes_read > 0, "Expected to read some data.");
    buffer[bytes_read] = '\0';
    // Check if the content matches what was written in setUp
    TEST_ASSERT_MESSAGE(strstr(buffer, "Hello!") != NULL, "Expected 'Hello!' in file content.");

    platform_fclose(fh);
}

// Test seeking and telling
void test_platform_fseek_ftell(void) {
    platform_file_handle fh = platform_fopen(test_filename, "rb");
    TEST_ASSERT_NOT_NULL_MESSAGE(fh, "Failed to open file for seeking test.");

    // Read first line and confirm position changes
    char buffer[32];
    memset(buffer, 0, sizeof(buffer));
    size_t bytes_read = platform_fread(buffer, 1, 6, fh); // "Hello!"
    TEST_ASSERT_EQUAL_UINT_MESSAGE(6, bytes_read, "Expected to read 6 bytes for 'Hello!");
    TEST_ASSERT_EQUAL_INT_MESSAGE(6, platform_ftell(fh), "Expected file position to be 6 after reading 6 bytes");

    // Seek back to start
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, platform_fseek(fh, 0, SEEK_SET), "platform_fseek to start failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, platform_ftell(fh), "Expected file position to be 0 after seeking to start.");

    platform_fclose(fh);
}

// Test opening a non-existent file
void test_platform_fopen_nonexistent(void) {
    platform_file_handle fh = platform_fopen("nonexistent_file.txt", "rb");
    TEST_ASSERT_NULL_MESSAGE(fh, "Expected NULL for non-existent file.");
}

// Test writing to a file
void test_platform_fwrite(void) {
    // Open a file for writing
    platform_file_handle fh = platform_fopen(test_filename, "wb");
    TEST_ASSERT_NOT_NULL_MESSAGE(fh, "Failed to open file for writing.");

    const char *data = "Some new content";
    size_t written = platform_fwrite(data, 1, strlen(data), fh);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(strlen(data), written, "Failed to write all data.");

    platform_fclose(fh);

    // Verify by reading back
    fh = platform_fopen(test_filename, "rb");
    TEST_ASSERT_NOT_NULL_MESSAGE(fh, "Failed to reopen file after writing.");
    char buffer[64];
    size_t bytes_read = platform_fread(buffer, 1, sizeof(buffer)-1, fh);
    buffer[bytes_read] = '\0';
    TEST_ASSERT_EQUAL_STRING_MESSAGE(data, buffer, "Read content differs from written content.");

    platform_fclose(fh);
}

#ifdef _WIN32
void test_unicode_paths(void) {
    // Test a filename with unicode chars
    // We'll create a file with a unicode name.
    // For simplicity, we pick a known safe unichode char e.g. U+00E9 (é)

    char utf8name[512];
    snprintf(utf8name, sizeof(utf8name), "%s/subdir/\xC3\xA9test.txt", temp_dir); // "étest.txt"

    platform_file_handle fh = fopen(utf8name, "wb");
    TEST_ASSERT_NOT_NULL_MESSAGE(fh, "Failed to create unicode test file");
    fputs("Data", fh);
    fclose(fh);

    // Now open with platform_fopen
    platform_file_handle fh2 = platform_fopen(utf8name, "rb");
    TEST_ASSERT_NOT_NULL_MESSAGE(fh2, platform_get_last_error());
    fseek(fh2, 0, SEEK_END);
    long sz = ftell(fh2);
    TEST_ASSERT_EQUAL_INT(4, (int)sz);
    fclose(fh2);
}
#endif

// Test error setting
void test_error_messages(void) {
    // Call something with invalid args
    int rc = platform_stat_file(NULL, NULL);
    TEST_ASSERT_NOT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_EQUAL('\0', platform_get_last_error()[0]);
}



