#ifndef FILE_LIST_H
#define FILE_LIST_H

#include <stddef.h>

typedef struct {
    char path[512];
    size_t size;
    int is_dir;
} file_info_t;

typedef struct {
    file_info_t *files;
    size_t count;
    size_t capacity;
} file_list_t;

// Initialize the file list
void file_list_init(file_list_t *list);

// Append a file_info_t entry to the list
void file_list_append(file_list_t *list, const file_info_t *info);

// Free the list resources
void file_list_free(file_list_t *list);

#endif  // FILE_LIST_H
