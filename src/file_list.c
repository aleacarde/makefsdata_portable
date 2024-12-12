#include "file_list.h"
#include <stdlib.h>
#include <string.h>

void file_list_init(file_list_t *list) {
    list->count = 0;
    list->capacity = 10;
    list->files = (file_info_t*)malloc(list->capacity * sizeof(file_info_t));
}

void file_list_append(file_list_t *list, const file_info_t *info) {
    if (list->count == list->capacity) {
        list->capacity *= 2;
        list->files = (file_info_t*)realloc(list->files, list->capacity * sizeof(file_info_t));
    }
    list->files[list->count++] = *info;
}

void file_list_free(file_list_t *list) {
    free(list->files);
    list->files = NULL;
    list->count = 0;
    list->capacity = 0;
}
