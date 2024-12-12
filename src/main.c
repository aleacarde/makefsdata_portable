#include "makefsdata_portable.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "file_list.h"
#include "scan.h"
#include "convert.h"

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

    for (size_t i = 0; i < list.count; i++) {
        file_info_t *finfo = &list.files[i];

        // Read file contents
        size_t file_size;
        unsigned char *data = convert_read_file_contents(finfo->path, &file_size);
        if (!data) {
            fprintf(stderr, "Failed to read file: %s\n", finfo->path);
            continue; // Skip this file
        }

        // Generate a variable name from the file index
        char var_name[64];
#ifdef _WIN32
        snprintf(var_name, sizeof(var_name), "file_%llu", i);
#else
        snprintf(var_name, sizeof(var_name), "file_%zu", i);
#endif

        // Until Generate.c is implemented, we will write the array to stdout for now
        convert_write_c_array(var_name, data, file_size, stdout);

        free(data);
    }

    // TODO: Generate fsdata.c with generate.c

    file_list_free(&list);
    return EXIT_SUCCESS;
}
