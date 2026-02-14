#pragma once

#include "common.h"

char *make_file_path(char *dir, char *file, char *ext);

/**
 * Callback function for directory scanning
 * @param filepath Full path to the file
 * @param userdata User-provided data pointer
 * @return 0 to continue, non-zero to stop scanning
 */
typedef int (*file_callback_t)(const char *filepath, void *userdata);

/**
 * Scan directory for files matching patterns
 * @param dir_path Directory path to scan
 * @param recursive Whether to scan subdirectories
 * @param callback Callback function to call for each file
 * @param userdata User data to pass to callback
 * @return Number of files processed, or -1 on error
 */
int scan_directory(const char *dir_path, int recursive, file_callback_t callback, void *userdata);

#ifdef WIN32
char *basename(char *path);
char *dirname(char *path);
#endif
