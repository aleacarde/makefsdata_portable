#include "scan.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>

static int scan_single_dir(const char *dir, const config_t *config, file_list_t *list) {
    platform_dir_handle *dh = platform_opendir(dir);
    if (!dh) {
        // platform_set_error was likely called by platform_opendir
        return -1; 
    }

    platform_file_info info;
    while (platform_readdir(dh, &info) == 0) {
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, info.name);

        if (info.is_dir) {
            if (config->recursive) {
                // Recursively scan subdirectories
                if (scan_single_dir(fullpath, config, list) != 0) {
                    // TODO: If an error occurs in a subdirectory, should we continue the operation or fail it?
                }
            }
        } else {
            // It's a file, add to the list
            file_info_t fi;
            strncpy(fi.path, fullpath, sizeof(fi.path)-1);
            fi.path[sizeof(fi.path)-1] = '\0';
            fi.size = info.size;
            fi.is_dir = 0;

            file_list_append(list, &fi);
        }
    }

    platform_closedir(dh);
    return 0;
}

int scan_directory(const char *dir, const config_t *config, file_list_t *list) {
    return scan_single_dir(dir, config, list);
}
