#ifndef SCAN_H
#define SCAN_H

#include "config.h"
#include "file_list.h"

// Recursively scan the directory specified in config.input_dir and populate the list.
// If config.recursive is true, also scan subdirectories.
int scan_directory(const char *dir, const config_t *config, file_list_t *list);

#endif // SCAN_H
