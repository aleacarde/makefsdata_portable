#include "config.h"
#include <stdio.h>
#include <string.h>

// Print usage instructions
static void print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf(" --input <dir>     Specify the input directory of web files.\n");
    printf(" --output <file>   Specify the output file for fsdata (e.g., fsdata.c).\n");
    printf(" --recursive       Recurse into subdirectories.\n");
    printf(" --help            Show this help message and exit.\n"); 
}

// Simple custom argument parser
bool parse_args(int argc, char **argv, config_t *config) {
    // Set defautls
    memset(config, 0, sizeof(*config));
    // You can set default paths or leave them empty for mandatory argument

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            config->show_help = true;
            return true; // Returning early, as help is a top-level action.
        } else if (strcmp(argv[i], "--input") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --input requires a directory argument.\n");
                return false;
            }
            strncpy(config->input_dir, argv[i + 1], sizeof(config->input_dir) - 1);
            i++; // Skip next argument since it's consumed by --input
        } else if (strcmp(argv[i], "--output") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --output requires a file argument.\n");
                return false;
            }
            strncpy(config->output_file, argv[i + 1], sizeof(config->output_file) - 1);
            i++; // Skip next argument since it's consumed by --output
        } else if (strcmp(argv[i], "--recursive") == 0) {
            config->recursive = true;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return false;
        }
    }

    // Validate mandatory arguments
    if (config->input_dir[0] == '\0') {
        fprintf(stderr, "Error: --input <dir> is required.\n");
        return false;
    }
    if (config->output_file[0] == '\0') {
        fprintf(stderr, "Error: --output <file> is required.\n");
        return false;
    }

    return true;
}

void print_help_message(const char *program_name) {
    print_usage(program_name);
}
