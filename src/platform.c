#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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
static int platform_convert_path_to_wchar(const char *path, WCHAR **wpath){
    *wpath = NULL;
    int wlen = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    if (wlen == 0) {
        platform_set_error("Failed to convert path to wchar: %s", path);
        return -1;
    }

    *path = (WCHAR*)malloc(wlen * sizeof(WCHAR));
    if (!*wpath) {
        platform_set_error("Out of memory");
        return -1;
    }

    if (MultiByteToWideChar(CP_UTF8, 0, path, -1, *wpath, wlen) == 0) {
        platform_set_error("Failed to convert path to wchar: %s", path);
        free(*wpath);
        *wpath = NULL;
        return -1;
    }

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
    WIN32_FIND_DATAA fdata;
    int first;
    char searchPath[1024];
#else
    DIR *d;
#endif
};

platform_dir_handle* platform_opendir(const char *path) {

#ifdef _WIN32
    platform_dir_handle *dh = (platform_dir_handle*)malloc(sizeof(platform_dir_handle));
    if (!dh) {
        platform_set_error("Out of memory");
        return NULL;
    }
    memset(dh, 0, sizeof(*dh));

    // Construct "path/*"
    size_t plen = strlen(path);
    char temp[1024];
    if (plen + 3 > sizeof(temp)) {
        platform_set_error("Path too long");
        free(dh);
        return NULL;
    }
    snprintf(temp, sizeof(temp), "%s%c*", path, PLATFORM_PATH_SEP);

    WCHAR *wsearch = NULL;
    if (platform_convert_path_to_wchar(temp, &wsearch) != 0) {
        free(dh);
        return NULL;
    }

    dh->searchPath = wsearch;
    dh->hFind = FindFirstFileW(dh->searchpath, &dh->fdata);
    if (dh->hFind == INVALID_HANDLE_VALUE) {
        platform_set_error("Failed to open directory: %s", path);
        free(dh->searchPath);
        free(dh);
        return NULL;
    }
    dh->first = 1;
#else
    DIR *d = opendir(path);
    if (!d) {
        platform_set_error("Failed to open directory: %s (errno=%d)", path, errno);
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
            // No more entries
            return -1;
        }
    } else {
        dh->first = 0;
    }

    // Skip "." and ".."
    if (wcscmp(fdata->cFileName, L".") == 0 || wcscmp(fdata->cFileName, L"..") == 0) {
        return platform_readdir(dh, info);
    }

    // Convert wchar filename back to UTF-8
    int needed = WideCharToMultiByte(CP_UTF8, 0, fdata->cFileName, -1, NULL, 0, NULL, NULL);
    if (needed <= 0) {
        platform_set_error("Failed to convert filename to UTF-8");
        return -1;
    }

    char *fname = (char*)malloc(needed);
    if (!fname) {
        platform_set_error("Out of memory");
        return -1;
    }

    if (WideCharToMultiByte(CP_UTF8, 0, fdata->cFileName, -1, fname, needed, NULL, NULL) == 0) {
        platform_set_error("Failed to convert filename to UTF-8");
        free(fname);
        return -1;
    }

    info->name = fname; // caller should free after use
    info->is_dir = (fdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;

    if (!info->is_dir) {
        LARGER_INTEGER size;
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
    free(dh->searchPath);
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

    WIN32_FILE_ATTRIBTE_DATA fad;
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

FILE* platform_fopen(const char *path) {
#ifdef _WIN32
    WCHAR *wpath = NULL;
    if (platform_convert_path_to_wchar(path, &wpath) != 0) {
        return NULL; // error set
    }
    FILE *fp = NULL;
    errno_t = _wfopen_s(&fp, wpath, L"rb");
    free(wpath);
    if (err != 0 || !fp) {
        platform_set_error("Failed to open file: %s", path);
        return NULL;
    }
    return fp;
#else
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        platform_set_error("Failed to open file: %s (errno=%d)", path, errno);
    }
    return fp;
#endif
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
