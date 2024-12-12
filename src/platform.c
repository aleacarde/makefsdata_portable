#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <wchar.h>

static char g_platform_error[256]; // Global error buffer

static void platform_set_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(g_platform_error, sizeof(g_platform_error), fmt, args);
    va_end(args);
}

const char* platform_get_last_error(void) {
    return g_platform_error[0] ? g_platform_error : "";
}

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <direct.h>

// Convert UTF-8 path to wide char (UTF-16)
// Returns 0 on success, nonzero on error.
static int platform_convert_path_to_wchar(const char *input_path, WCHAR **wpath){
    if (!input_path || !wpath) {
        platform_set_error("Invalid arguments to platform_convert_path_to_wchar\n");
        return -1;
    }

    // Convert input path to wide characters
    size_t len = strlen(input_path) + 1; // +1 for null terminator
    size_t wlen = mbstowcs(NULL, input_path, 0) + 1; // Get required wide-char length

    if (wlen == len-1) {
        platform_set_error("Error converting input path to wide characters\n");
        return -1;
    }

    *wpath = (WCHAR*)malloc(wlen * sizeof(WCHAR)); // Allocate memory for WCHAR string
    if (!*wpath) {
        platform_set_error("Memory allocation failed in platform_convert_path_to_wchar\n");
        return -1;
    }

    mbstowcs(*wpath, input_path, wlen); // Perform conversion

    return 0;
}

#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#endif

struct platform_dir_handle {
#ifdef _WIN32
    HANDLE hFind;
    WIN32_FIND_DATAW fdata;
    int first;
    char searchPath[MAX_PATH_LENGTH];
#else
    DIR *d;
#endif
};

platform_dir_handle* platform_opendir(const char *searchPath) {

#ifdef _WIN32
    platform_dir_handle *dh = (platform_dir_handle*)malloc(sizeof(platform_dir_handle));
    if (!dh) {
        platform_set_error("Out of memory");
        return NULL;
    }
    memset(dh, 0, sizeof(*dh));

    // Ensure searchPath fits in buffer
    if (strlen(searchPath) + 2 > MAX_PATH_LENGTH) {
        platform_set_error("Search path is too long");
        free(dh);
        return NULL;
    }

    // Copy and normalize searchPath
    snprintf(dh->searchPath, MAX_PATH_LENGTH, "%s", searchPath);
    size_t len = strlen(dh->searchPath);
    if (len > 0 && dh->searchPath[len - 1] == '\\') {
        dh->searchPath[len - 1] = '\0';  // Remove trailing backslash
    }
    strncat(dh->searchPath, "\\*", MAX_PATH_LENGTH - strlen(dh->searchPath) - 1);

    // Convert searchPath to wide characters
    wchar_t wideSearchPath[MAX_PATH_LENGTH];
    size_t convertedChars = 0;
    if (mbstowcs_s(&convertedChars, wideSearchPath, MAX_PATH_LENGTH, dh->searchPath, _TRUNCATE) != 0) {
        platform_set_error("Failed to convert searchPath to WCHAR");
        free(dh);
        return NULL;
    }

    // Perform the first directory search
    dh->hFind = FindFirstFileW(wideSearchPath, &dh->fdata);
    if (dh->hFind == INVALID_HANDLE_VALUE) {
        platform_set_error("Failed to open directory: %s", searchPath);
        free(dh);
        return NULL;
    }
    dh->first = 1;
#else
    DIR *d = opendir(searchPath);
    if (!d) {
        platform_set_error("Failed to open directory: %s (errno=%d)", searchPath, errno);
        return NULL;
    }
    platform_dir_handle *dh = (platform_dir_handle*)malloc(sizeof(platform_dir_handle));
    if (!dh) {
        platform_set_error("Out of memory");
        closedir(d);
        return NULL;
    }
    dh->d = d;
#endif

    return dh;
}


int platform_readdir(platform_dir_handle *dh, platform_file_info *info){
    if (!dh || !info) {
        platform_set_error("Invalid arguments to platform_readdir");
        return -1;
    }

#ifdef _WIN32
    // On Windows
    WIN32_FIND_DATAW *fdata = &dh->fdata;
    if (!dh->first) {
        if (!FindNextFileW(dh->hFind, fdata)) {
            printf("No more entries found\n");
            return -1;
        }
    } else {
        dh->first = 0;
    }

    // Debug: Print the retrieved entry
    wprintf(L"Retrieved: %ls\n", fdata->cFileName);

    // Skip "." and ".."
    if (wcscmp(fdata->cFileName, L".") == 0 || wcscmp(fdata->cFileName, L"..") == 0) {
        return platform_readdir(dh, info);
    }

    if (WideCharToMultiByte(CP_UTF8, 0, fdata->cFileName, -1, info->name, MAX_FILENAME_LENGTH, NULL, NULL) == 0) {
        platform_set_error("Failed to convert filename to UTF-8");
        return -1;
    }

    info->is_dir = (fdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
    printf("Processing entry: %ls\n", fdata->cFileName);
    printf("Attributes: 0x%lu, is_dir: %d\n", fdata->dwFileAttributes, info->is_dir);

    if (!info->is_dir) {
        LARGE_INTEGER size;
        size.LowPart = fdata->nFileSizeLow;
        size.HighPart = fdata->nFileSizeHigh;
        info->size = (size_t) size.QuadPart;
    } else {
        info->size = 0;
    }

#else
    // On POSIX (macOS/Linux)
    struct dirent *entry = readdir(dh->d);
    if (entry == NULL) {
        // No more entries
        return -1;
    }

    // Skip "." and ".."
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
        return platform_readdir(dh, info);
    }

    strncpy(info->name, entry->d_name, MAX_FILENAME_LENGTH - 1);
    info->name[MAX_FILENAME_LENGTH - 1] = '\0';

    // Determine if directory using stat
    struct stat st;
    if (fstatat(dirfd(dh->d), entry->d_name, &st, 0) == -1) {
        platform_set_error("Failed to stat directory entry: %s (errno=%d)", entry->d_name, errno);
        return -1;
    }

    if (S_ISDIR(st.st_mode)) {
        info->is_dir = 1;
        info->size = 0;
    } else {
        info->is_dir = 0;
        info->size = (size_t)st.st_size;
    }
#endif

    return 0;
}

int platform_closedir(platform_dir_handle *dh) {
    if (!dh) {
        platform_set_error("Invalid directory handle");
        return -1;
    }
#ifdef _WIN32
    FindClose(dh->hFind);
#else
    closedir(dh->d);
#endif
    free(dh);
    return 0;
}

int platform_stat_file(const char *path, platform_file_info *info) {
    if (!path || !info) {
        platform_set_error("Invalid arguments to platform_stat_file");
        return -1;
    }

#ifdef _WIN32
    WCHAR *wpath = NULL;
    if (platform_convert_path_to_wchar(path, &wpath) != 0) {
        return -1; // error already set
    }

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExW(wpath, GetFileExInfoStandard, &fad)) {
        platform_set_error("Failed to stat file: %s", path);
        free(wpath);
        return -1;
    }

    free(wpath);

    if (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        info->is_dir = 1;
        info->size = 0;
    } else {
        info->is_dir = 0;
        LARGE_INTEGER size;
        size.LowPart = fad.nFileSizeLow;
        size.HighPart = fad.nFileSizeHigh;
        info->size = (size_t) size.QuadPart;
    }

#else
    struct stat st;
    if (stat(path, &st) == -1) {
        platform_set_error("Failed to stat file: %s (errno=%d)", path, errno);
        return -1;
    }

    if (S_ISDIR(st.st_mode)) {
        info->is_dir = 1;
        info->size = 0;
    } else {
        info->is_dir = 0;
        info->size = (size_t)st.st_size;
    }

#endif
    return 0;
}

platform_file_handle platform_fopen(const char *path, const char *mode) {
#ifdef _WIN32
    WCHAR *wpath = NULL;
    if (platform_convert_path_to_wchar(path, &wpath) != 0) {
        return NULL; // error set
    }

    // Convert `mode` to WCHAR
    // We'll assume `mode` is short and ASCII-only, so mbstowcs_s is safe.

    WCHAR wmode[32];
    size_t convertedChars = 0;
    if (mbstowcs_s(&convertedChars, wmode, _countof(wmode), mode, _TRUNCATE) != 0) {
        platform_set_error("Failed to convert mode to WCHAR");
        free(wpath);
        return NULL;
    }

    platform_file_handle fh = NULL;
    errno_t err = _wfopen_s(&fh, wpath, wmode);
    free(wpath);

    if (err != 0 || !fh) {
        platform_set_error("Failed to open file: %s", path);
        return NULL;
    }
#else
    platform_file_handle fh = fopen(path, mode);
    if (!fh) {
        platform_set_error("Failed to open file: %s (errno=%d)", path, errno);
    }
#endif

return fh;
}

int platform_fclose(platform_file_handle fh) {
    return fclose(fh);
}

size_t platform_fread(void *ptr, size_t size, size_t nmemb, platform_file_handle fh) {
    return fread(ptr, size, nmemb, fh);
}

size_t platform_fwrite(const void *ptr, size_t size, size_t nmemb, platform_file_handle fh) {
    return fwrite(ptr, size, nmemb, fh);
}

int platform_fseek(platform_file_handle fh, long offset, int whence) {
    return fseek(fh, offset, whence);
}

long platform_ftell(platform_file_handle fh) {
    return ftell(fh);
}

void platform_normalize_path(char *path, size_t path_len) {
    // Optional: For windows, you might want to convert '/' to '\\'.
#ifdef _WIN32
    for (size_t i = 0; i < path_len && path[i] != '\0'; i++) {
        if (path[i] == '/') {
            path[i] = '\\';
        }
    }
#else
    // On POSIX systems, do nothing
    (void)path;
    (void)path_len;
#endif
}
