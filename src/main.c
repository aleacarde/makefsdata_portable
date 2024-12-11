#include "makefsdata_portable.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"


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

    printf("Input: %s\n", config.input_dir);
    printf("Output: %s\n", config.output_file);
    printf("Recursive: %d\n", (int)config.recursive);

    printf("There will be stuff here.\n");
    
    return EXIT_SUCCESS;
}
