#include "unity.h"
#include "test_shared.h"
#include "config.h"
#include "file_list.h"
#include "scan.h"
#include <string.h>


// Test scanning an empty directory
void test_scan_empty_dir(void) {
    const char *root = TEST_RESOURCES_DIR;
    snprintf(config.input_dir, sizeof(config.input_dir), "%s/empty_dir", root);
    config.recursive = 0;

    int result = scan_directory(config.input_dir, &config, &list);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_UINT64(0, list.count); // No files
}

// Test scanning a directory with a single file
void test_scan_single_file(void) {
    const char *root = TEST_RESOURCES_DIR;
    snprintf(config.input_dir, sizeof(config.input_dir), "%s/single_file", root);
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/single_file/file1.txt", root);
    config.recursive = 0;

    int result = scan_directory(config.input_dir, &config, &list);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_UINT64(1, list.count);
    TEST_ASSERT_EQUAL_STRING(filePath, list.files[0].path);
    TEST_ASSERT(list.files[0].size > 0); // file1.txt should have content
    TEST_ASSERT_EQUAL(0, list.files[0].is_dir);
}

// Test scanning a directory with subdirectories without recursion
void test_scan_subdirs_no_recursion(void) {
    const char *root = TEST_RESOURCES_DIR;
    snprintf(config.input_dir, sizeof(config.input_dir), "%s/subdirs", root);
    char file2Path[256];
    snprintf(file2Path, sizeof(file2Path), "%s/subdirs/file2.txt", root);
    config.recursive = 0;

    int result = scan_directory(config.input_dir, &config, &list);
    TEST_ASSERT_EQUAL(0, result);
    // The top-level directory has file2.txt and a subdir named "subdir"
    // Without recursion, we should only see file2.txt
    TEST_ASSERT_EQUAL_UINT64(1, list.count);
    TEST_ASSERT_EQUAL_STRING(file2Path, list.files[0].path);
}

// Test scanning a directory with subdirectories with recursion.
void test_scan_subdirs_with_recursion(void) {
    const char *root = TEST_RESOURCES_DIR;
    snprintf(config.input_dir, sizeof(config.input_dir), "%s/subdirs", root);
    char file2Path[256];
    char file3Path[256];
    snprintf(file2Path, sizeof(file2Path), "%s/subdirs/file2.txt", root);
    snprintf(file3Path, sizeof(file3Path), "%s/subdirs/subdir/file3.txt", root);
    config.recursive = 1;

    int result = scan_directory(config.input_dir, &config, &list);
    TEST_ASSERT_EQUAL(0, result);
    // Wih recursion, we should see file2.txt and file3.txt inside subdir
    // The order is not guaranteed, so we only check counts and existence
    TEST_ASSERT_EQUAL_UINT64(2, list.count);

    // Check files by searching through the list
    int found_file2 = 0;
    int found_file3 = 0;
    for (size_t i = 0; i < list.count; i++) {
        if (strcmp(list.files[i].path, file2Path) == 0) {
            found_file2 = 1;
        } else if (strcmp(list.files[i].path, file3Path) == 0) {
            found_file3 = 1;
        }
    }
    TEST_ASSERT_TRUE(found_file2);
    TEST_ASSERT_TRUE(found_file3);
}
