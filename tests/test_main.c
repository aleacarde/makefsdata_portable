#include "unity.h"

extern void setUp(void);
extern void tearDown(void);
extern void test_directory_ops(void);
extern void test_directory_error(void);
extern void test_stat_file(void);
extern void test_fopen(void);
extern void test_unicode_paths(void);
extern void test_error_messages(void);

int main(void) {
    UNITY_BEGIN();

    // Set up test environment
    setUp();

    RUN_TEST(test_directory_ops);
    RUN_TEST(test_directory_error);
    RUN_TEST(test_stat_file);
    RUN_TEST(test_fopen);
#ifdef _WIN32
    RUN_TEST(test_unicode_paths);
#endif
    RUN_TEST(test_error_messages);

    // Cleanup
    tearDown();

    return UNITY_END();
}
