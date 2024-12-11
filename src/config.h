#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
    char input_dir[256];
    char output_file[256];
    bool recursive;
    bool show_help;
} config_t;

bool parse_args(int argc, char **argv, config_t *config);
void print_help_message(const char *program_name);

#endif // CONFIG_H
