#include "test_shared.h"

// Global Test State
char temp_dir[256];
config_t config;
file_list_t list;
const char *test_filename = "test_file.txt";
const char *nonempty_filename = "test_nonempty.txt";
const char *empty_filename = "test_empty.txt";
const char *nonexistent_filename = "no_such_file.txt";
const char *large_filename = "test_large.txt";
