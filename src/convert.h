#ifndef CONVERT_H
#define CONVERT_H

#include <stddef.h>
#include <stdio.h>
#include "platform.h"

// Reads the entire file at 'path' into a newly allocated buffer.
// Returns pointer to the buffer, and writes its size into *size_out.
// Caller must free the returned buffer.
unsigned char* convert_read_file_contents(const char *path, size_t *size_out);

// Writes the file's data as a static const unsigned char array into a given output stream.
// var_name: The C identifier to use for the array variable.
void convert_write_c_array(const char *var_name, const unsigned char *data, size_t size, platform_file_handle out);



#endif // CONVERT_H
