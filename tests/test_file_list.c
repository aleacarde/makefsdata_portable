#include "file_list.h"
#include "test_shared.h"
#include "unity.h"
#include <string.h>

void test_file_list_init(void) {
    TEST_ASSERT_EQUAL_UINT64(0, list.count);
    TEST_ASSERT_TRUE(list.capacity > 0);
    TEST_ASSERT_NOT_NULL(list.files);
}

void test_file_list_append_single(void) {
    file_info_t fi;
    strncpy(fi.path, "testpath", sizeof(fi.path)-1);
    fi.path[sizeof(fi.path)-1] = '\0';
    fi.size = 123;
    fi.is_dir = 0;

    file_list_append(&list, &fi);

    TEST_ASSERT_EQUAL_UINT64(1, list.count);
    TEST_ASSERT_EQUAL_STRING("testpath", list.files[0].path);
    TEST_ASSERT_EQUAL_UINT64(123, list.files[0].size);
    TEST_ASSERT_EQUAL(0, list.files[0].is_dir);
}

void test_file_list_append_multiple(void) {
    for (int i = 0; i < 25; i++) {
        file_info_t fi;
        snprintf(fi.path, sizeof(fi.path), "file%d", i);
        fi.size = i * 10;
        fi.is_dir = (i % 2 == 0) ? 1 : 0;
        file_list_append(&list, &fi);
    }

    TEST_ASSERT_EQUAL_UINT64(25, list.count);
    // Check a few entries
    TEST_ASSERT_EQUAL_STRING("file0", list.files[0].path);
    TEST_ASSERT_EQUAL(0, list.files[0].size);
    TEST_ASSERT_EQUAL(1, list.files[0].is_dir); // Even => is_dir = 1

    TEST_ASSERT_EQUAL_STRING("file24", list.files[24].path);
    TEST_ASSERT_EQUAL(240, list.files[24].size);
    TEST_ASSERT_EQUAL(1, list.files[24].is_dir); // Even => is_dir = 1
}

void test_file_list_free(void) {
    // Adding a few entries before freeing
    file_info_t fi;
    strcpy(fi.path, "dummy");
    fi.size = 100;
    fi.is_dir = 0;
    file_list_append(&list, &fi);

    file_list_free(&list);
    TEST_ASSERT_NULL(list.files);
    TEST_ASSERT_EQUAL_UINT64(0, list.count);
    TEST_ASSERT_EQUAL_UINT64(0, list.capacity);
}
