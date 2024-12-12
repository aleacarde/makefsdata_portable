#ifndef TEST_SHARED_H
#define TEST_SHARED_H

#include "config.h"
#include "file_list.h"

extern char temp_dir[256];
extern config_t config;
extern file_list_t list;
extern const char *test_filename;
extern const char *nonempty_filename;
extern const char *empty_filename;
extern const char *nonexistent_filename;
extern const char *large_filename;

#endif // TEST_SHARED_H
