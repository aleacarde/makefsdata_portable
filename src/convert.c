#include "convert.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* convert_read_file_contents(const char *path, size_t *size_out) {
    // Open file in binary mode
    platform_file_handle fh = platform_fopen(path, "rb");
    if (!fh) {
        return NULL; // Caller can handle error
    }

    // Determine file size
    if (platform_fseek(fh, 0, SEEK_END) != 0) {
        platform_fclose(fh);
        return NULL;
    }
    long size = platform_ftell(fh);
    if (size < 0) {
        platform_fclose(fh);
        return NULL;
    }

    // Move back to start
    platform_fseek(fh, 0, SEEK_SET);

    // Allocate buffer
    unsigned char *buffer = (unsigned char*)malloc((size_t)size);
    if (!buffer) {
        platform_fclose(fh);
        return NULL;
    }

    size_t read_count = platform_fread(buffer, 1, (size_t)size, fh);
    platform_fclose(fh);

    if (read_count != (size_t)size) {
        free(buffer);
        return NULL;
    }

    *size_out = (size_t)size;
    return buffer;
}

void convert_write_c_array(const char *var_name, const unsigned char *data, size_t size, platform_file_handle out) {
    fprintf(out, "static const unsigned char %s[] = {\n", var_name);

    for (size_t i = 0; i < size; i++) {
        fprintf(out, "0x%02X,", data[i]);
        if ((i + 1) % 16 == 0) {
            fprintf(out, "\n");
        }
    }
    fprintf(out, "\n};\n\n");
}
