#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdio.h>

#define MAX_FILENAME_LENGTH 256

// This structure holds information about a directory entry (file or directory).
typedef struct {
    char name[MAX_FILENAME_LENGTH];         // The filename. Max filename length in macOS and linux is 255 bytes 
    int is_dir;         // 1 if directory, 0 if regualr file.
    size_t size;        // Size in bytes if a file, 0 if directory
} platform_file_info;

// Opaque handle for directory iteration
typedef struct platform_dir_handle platform_dir_handle;

// Open a directory for iteration
// Returns NULL on failure.
platform_dir_handle* platform_opendir(const char *path);

// Read the next entry in the directory.
// On sucess fills `info` with details of the entry and returns 0.
// On error or no more entires, returns nonzero.
// The caller may copy data from info->name before calling again
// since subsuqeuent calls may overwrite the buffer 
int platform_readdir(platform_dir_handle *dh, platform_file_info *info);

// Close the directory handle.
int platform_closedir(platform_dir_handle *dh);

// Get information about a specific file.
// Returns 0 on success, nonzero on error.
int platform_stat_file(const char *path, platform_file_info *info);

// Open a file for reading (binary mode).
// Returns a FILE* pointer or NULL on error.
FILE* platform_fopen(const char *path);

// The caller uses fread/fclose from standard C library to read and close files.
// This is acceptable since C standard I/O is portable enough.

// Function to normalize a path if needed (optional):
// On windows you might convert '/' to '\\', for example.
// Here we keep it simple and may just return the input or do minimal changes.
void platform_normalize_path(char *path, size_t path_len);

// Get the last error message.
// Returns a pointer to a static buffer containing the last error message set by
// playform functions.
const char* platform_get_last_error(void);

// Path separator character
#ifdef _WIN32
#define PLATFORM_PATH_SEP '\\'
#else
#define PLATFORM_PATH_SEP '/'
#endif

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_H
