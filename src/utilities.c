#include "utilities.h"
#include <sys/stat.h>

#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

char *make_file_path(char *dir, char *file, char *ext) {
  uint32_t file_path_length = strlen(dir) + strlen(file) + strlen(ext) + 2;
  char *file_path = malloc(file_path_length);
  memset(file_path, 0, file_path_length);
  _STRCPY(file_path, dir, file_path_length);
  _STRCAT(file_path, "/", file_path_length);
  _STRCAT(file_path, file, file_path_length);
  _STRCAT(file_path, ext, file_path_length);

  return file_path;
}

#ifdef WIN32
char *basename(char *path) {
  uint32_t start = 0;
  uint32_t len = strlen(path);

  for (uint32_t i = 0; path[i] != 0; ++i) {
    if ((path[i] == '/' || path[i] == '\\') && (i < (len - 1))) {
      start = i + 1;
    }
  }

  return path + start;
}

char *dirname(char *path) {
  assert(PathCchRemoveFileSpec(path, strlen(path)) == S_OK);
  return path;
}
#endif

static int has_cgfx_extension(const char *filename) {
  size_t len = strlen(filename);
  
  // Check for .cgfx or .bin extension
  if (len > 5 && strcmp(filename + len - 5, ".cgfx") == 0) {
    return 1;
  }
  if (len > 4 && strcmp(filename + len - 4, ".bin") == 0) {
    return 1;
  }
  
  return 0;
}

int scan_directory(const char *dir_path, int recursive, file_callback_t callback, void *userdata) {
  int count = 0;
  
#ifdef WIN32
  WIN32_FIND_DATA find_data;
  HANDLE hFind;
  char search_path[MAX_PATH];
  
  snprintf(search_path, MAX_PATH, "%s\\*", dir_path);
  hFind = FindFirstFile(search_path, &find_data);
  
  if (hFind == INVALID_HANDLE_VALUE) {
    return -1;
  }
  
  do {
    if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
      continue;
    }
    
    char full_path[MAX_PATH];
    snprintf(full_path, MAX_PATH, "%s\\%s", dir_path, find_data.cFileName);
    
    if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if (recursive) {
        int sub_count = scan_directory(full_path, recursive, callback, userdata);
        if (sub_count > 0) {
          count += sub_count;
        }
      }
    } else {
      if (has_cgfx_extension(find_data.cFileName)) {
        if (callback(full_path, userdata) == 0) {
          count++;
        } else {
          break;
        }
      }
    }
  } while (FindNextFile(hFind, &find_data) != 0);
  
  FindClose(hFind);
  
#else
  DIR *dir = opendir(dir_path);
  if (!dir) {
    return -1;
  }
  
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    
    char full_path[PATH_MAX];
    snprintf(full_path, PATH_MAX, "%s/%s", dir_path, entry->d_name);
    
    struct stat statbuf;
    if (stat(full_path, &statbuf) == 0) {
      if (S_ISDIR(statbuf.st_mode)) {
        if (recursive) {
          int sub_count = scan_directory(full_path, recursive, callback, userdata);
          if (sub_count > 0) {
            count += sub_count;
          }
        }
      } else if (S_ISREG(statbuf.st_mode)) {
        if (has_cgfx_extension(entry->d_name)) {
          if (callback(full_path, userdata) == 0) {
            count++;
          } else {
            break;
          }
        }
      }
    }
  }
  
  closedir(dir);
#endif
  
  return count;
}
