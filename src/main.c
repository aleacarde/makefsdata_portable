#include "makefsdata_portable.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "file_list.h"
#include "scan.h"

extern bool parse_args(int argc, char **argv, config_t *config);
extern void print_help_message(const char *program_name);


int main (int argc, char **argv) {
    config_t config;
    if (!parse_args(argc, argv, &config)) {
        fprintf(stderr, "Try --help for usage.\n");
        return EXIT_FAILURE;
    }

    if (config.show_help) {
        print_help_message(argv[0]);
        return EXIT_SUCCESS;
    }

    file_list_t list;
    file_list_init(&list);

    if (scan_directory(config.input_dir, &config, &list) != 0) {
        fprintf(stderr, "Failed to scan directory: %s\n", config.input_dir);
        file_list_free(&list);
        return EXIT_FAILURE;
    }

    // Print the files found
    for (size_t i = 0; i < list.count; i++) {
        file_info_t *f = &list.files[i];
#ifdef _WIN32
        printf("File: %s, Size: %lu\n", f->path, f->size);
#else
        printf("File: %s, Size: %zu\n", f->path, f->size);
#endif
    }

    // TODO: Convert files with convert.c
    // TODO: Generate fsdata.c with generate.c

    file_list_free(&list);
    return EXIT_SUCCESS;
}
