#include "convert.h"
#include "unity.h"
#include "test_shared.h"
#include "platform.h"
#include <string.h>
#include <stdlib.h>

// Test reading non-empty file
void test_convert_read_nonempty(void) {
    size_t size_out = 0;
    unsigned char *data = convert_read_file_contents(nonempty_filename, &size_out);
    TEST_ASSERT_NOT_NULL_MESSAGE(data, "Expected to read non-empty file.");
    TEST_ASSERT_MESSAGE(size_out > 0, "Size should be > 0 for non-empty file.");
    TEST_ASSERT_NOT_EQUAL(0, strstr((char*)data, "Hello convert!") != NULL);
    free(data);
}

// Test reading empty file
void test_convert_read_empty(void) {
    size_t size_out = 0;
    unsigned char *data = convert_read_file_contents(empty_filename, &size_out);
    if (data == NULL) {
        TEST_FAIL_MESSAGE("convert_read_file_contents returned NULL for empty file.");
    } else {
        TEST_ASSERT_EQUAL_UINT(0, size_out);
        free(data);
    }
}

// Test reading nonexistent file
void test_convert_read_nonexistent(void) {
    size_t size_out = 0;
    unsigned char *data = convert_read_file_contents(nonexistent_filename, &size_out);
    TEST_ASSERT_NULL_MESSAGE(data, "Nonexistent file should return NULL.");
}

// Test reading large file
void test_convert_read_large(void) {
    size_t size_out = 0;
    unsigned char *data = convert_read_file_contents(large_filename, &size_out);
    TEST_ASSERT_NOT_NULL_MESSAGE(data, "Reading large file failed.");
    TEST_ASSERT_EQUAL_UINT(1048576, size_out);
    // Check content
    for (size_t i = 0; i < size_out; i++) {
        TEST_ASSERT_EQUAL_MESSAGE('A', data[i], "Expected 'A' in large file content");
    }
    free(data);
}

// Test convert_write_c_array with known data
void test_convert_write_c_array_basic(void) {
    const unsigned char test_data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello in ASCII"
    size_t size = sizeof(test_data);

    // Use tmpfile() to capture output
    platform_file_handle out = tmpfile();
    TEST_ASSERT_NOT_NULL_MESSAGE(out, "Failed to create tmpfile for testing c array output.");

    convert_write_c_array("test_var", test_data, size, out);

    platform_fseek(out, 0, SEEK_SET);
    char buffer[256];
    size_t read_count = fread(buffer, 1, sizeof(buffer)-1, out);
    buffer[read_count] = '\0';

    // Check that we see "static const unsigned char test_var[] = {"
    // and "0x48, 0x65, 0x6C, 0x6C, 0x6F" in output
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(buffer, "static const unsigned char test_var[] = {"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(buffer, "0x48,0x65,0x6C,0x6C,0x6F"));
}
// Test convert_write_c_array with empty data
void test_convert_write_c_array_empty(void) {
    const unsigned char *empty_data = NULL;
    size_t size = 0;

    platform_file_handle out = tmpfile();
    TEST_ASSERT_NOT_NULL_MESSAGE(out, "tmpfile failed for empty data test.");

    convert_write_c_array("empty_var", empty_data, size, out);

    fseek(out, 0, SEEK_SET);
    char buffer[256];
    size_t read_count = platform_fread(buffer, 1, sizeof(buffer)-1, out);
    buffer[read_count] = '\0';

    TEST_ASSERT_NOT_EQUAL(NULL, strstr(buffer, "static const unsigned char empty_var[] = {"));
    // It should just have a newline after this and then "};"
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(buffer, "};"));
}
