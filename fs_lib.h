#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

/*
void debug_print(char* fmt, ...);
void print_errno(void);
int does_path_exist(char* path);
int is_path_dir(char* path);
int is_path_file(char* path);
int dlist_append(struct dirlist* dirlist, char* item);
void print_dlist(struct dirlist dirlist);
struct dirlist iterate_dir(char* path);
int create_file(char* path);
int delete_file(char* path);
int create_dir(char* path);
int remove_dir(char* path);
void* load_file_from_path(char* path);
int unload_file_from_memory(char* mem, unsigned size);
int create_file_recursively(char* path);
int create_dir_recursively(const char* const path);
int remove_dir_recursively(const char* const path);
*/

void debug_print(char* fmt, ...) {
#ifdef DEBUG
  printf("[DEBUG] ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);
#endif
  return;
}

void print_errno(void) {
  debug_print("%s\n", strerror(errno));
  return;
}

#define print_fn_entry debug_print("entered %s\n", __FUNCTION__)
#define print_fn_leave debug_print("leaving %s\n", __FUNCTION__)

int does_path_exist(char* path) {
  struct stat fi;
  if(stat(path, &fi) < 0) {
    if(errno == ENOENT) {
      // print_errno();
      return 0;
    }
  }
  return 1;
}

int is_path_dir(char* path) {
  if(!does_path_exist(path)) {
    debug_print("%s does not exist\n", path);
    return 0;
  }
  struct stat fi;
  if(stat(path, &fi) < 0) {
    debug_print("%s\n", strerror(errno));
    return 0;
  }
  if(S_ISDIR(fi.st_mode)) {
    return 1;
  }
  return 0;
}

int is_path_file(char* path) {
  if(!does_path_exist(path)) {
    debug_print("%s does not exist\n", path);
    return 0;
  }
  struct stat fi;
  if(stat(path, &fi) < 0) {
    debug_print("%s\n", strerror(errno));
    return 0;
  }
  if(S_ISREG(fi.st_mode)) {
    return 1;
  }
  return 0;
}

struct dirlist {
  char** dir;
  unsigned count;
  unsigned capacity;
};

int dlist_append(struct dirlist* dirlist, char* item) {
  if(dirlist->count == 0) {
    dirlist->dir = (char**)calloc(1, 8 * sizeof(char*));
    dirlist->capacity = 8;
  } else {
    dirlist->dir = (char**)realloc(dirlist->dir, (dirlist->count + 3) * sizeof(void*));
    if(dirlist->count >= dirlist->capacity) {
      dirlist->capacity *= 2;
      dirlist->dir = (char**)realloc(dirlist->dir, dirlist->capacity * sizeof(void*));
    }
  }
  if(dirlist->dir == NULL) {
    return 0;
  }
  dirlist->dir[dirlist->count] = item;
  dirlist->count += 1;
  dirlist->dir[dirlist->count] = NULL;
  return 1;
}

void print_dlist(struct dirlist dirlist) {
  for(int i = 0; i < dirlist.count; i++) {
    printf("%s\n", dirlist.dir[i]);
  }
  printf("dirlist.count=%d\n", dirlist.count);
}

struct dirlist iterate_dir(struct dirlist* dirlist, char* path) {
  struct dirlist null_list = {0};
  if(!does_path_exist(path)) {
    return null_list;
  }
  if(!is_path_dir(path)) {
    debug_print("%s is not a directory\n", path);
    return null_list;
  }
  DIR* dir = opendir(path);
  struct dirent* dirent;
  int path_len = strlen(path);
  char* file;
  for(int i = 0; (dirent = readdir(dir)) != NULL; i++) {
    if(strlen(dirent->d_name) <= 2 && dirent->d_name[0] == '.' || dirent->d_name[1] == '.') {
      continue;
    }
    int max_len = strlen(dirent->d_name) + path_len;
    file = (char*)calloc(1, PATH_MAX);
    snprintf(file, PATH_MAX, "%s/%s", path, dirent->d_name);
    if(is_path_dir(file)) {
      iterate_dir(dirlist, file);
    }
    dlist_append(dirlist, file);
  }
  return null_list;
}

int create_file(char* path) {
  if(!does_path_exist(path)) {
    if(creat(path, 0644) < 0) {
      print_errno();
      return 0;
    }
    debug_print("created file %s\n", path);
    return 1;
  }
  debug_print("%s already exists\n", path);
  return 0;
}

int create_dir(char* path) {
  if(!does_path_exist(path)) {
    if(mkdir(path, 0755) < 0) {
      print_errno();
      return 0;
    }
    debug_print("created dir %s\n", path);
    return 1;
  }
  debug_print("%s already exists\n", path);
  return 0;
}

int remove_file(char* path) {
  if(does_path_exist(path)) {
    if(unlink(path) < 0) {
      print_errno();
      return 0;
    }
    debug_print("deleted %s\n", path);
    return 1;
  }
  debug_print("%s does not exist\n", path);
  return 0;
}

int remove_dir(char* path) {
  if(does_path_exist(path)) {
    if(rmdir(path) < 0) {
      print_errno();
      return 0;
    }
    debug_print("removed %s\n", path);
    return 1;
  } else {
    debug_print("%s did not exist\n", path);
  }
  return 0;
}

__attribute__((warn_unused_result)) void* load_file_from_path(char* path, int prot, int flags) {
  struct stat fi;
  if(stat(path, &fi) < 0) {
    print_errno();
    return 0;
  }
  int fd_path = open(path, 0644);
  if(fd_path == -1) {
    print_errno();
    return 0;
  }
  void* file_mem = mmap(0, fi.st_size, prot, flags, fd_path, 0);
  if(file_mem == MAP_FAILED) {
    print_errno();
    return NULL;
  }
  close(fd_path);
  return file_mem;
}

int unload_file_from_memory(char* mem, unsigned size) {
  if(munmap(mem, size) < 0) {
    print_errno();
    return 0;
  }
  return 1;
}

int create_file_recursively(char* path) {
  if(does_path_exist(path)) {
    return 1;
  }
  int file_len = strlen(path);
  int i = 0;
  char* file = (char*)calloc(1, file_len);
  for(; i <= file_len; i++) {
    file[i] = path[i];
    if(file[i] == '/') {
      if(does_path_exist(file)) {
        continue;
      }
      create_dir(file);
    }
  }
  create_file(file);
  return 1;
}

int create_dir_recursively(const char* const path) {
  print_fn_entry;
  if(does_path_exist((char*)path)) {
    return 1;
  }
  int file_len = strlen(path);
  int i = 0;
  char* file = (char*)calloc(1, file_len);
  for(; i < file_len; i++) {
    file[i] = path[i];
    if(file[i] == '/') {
      if(does_path_exist(file)) {
        continue;
      }
      create_dir(file);
    }
  }
  create_dir(file);
  print_fn_leave;
  return 1;
}

int remove_dir_recursively(const char* const path) {
  print_fn_entry;
  if(!does_path_exist((char*)path)) {
    printf("%s does not exist\n", path);
    return 0;
  }
  struct dirent* dirent;
  unsigned plen = strlen(path);
  char buff[PATH_MAX];
  for(int i = 0; i < plen; i++) {
    if(path[i] == '/') {
      DIR* dir = opendir(path);
      while((dirent = readdir(dir)) != NULL) {
        if(strlen(dirent->d_name) <= 2 && dirent->d_name[0] == '.' || dirent->d_name[1] == '.') {
          continue;
        }
        snprintf(buff, PATH_MAX, "%s/%s", path, dirent->d_name);
        if(is_path_dir(buff)) {
          remove_dir_recursively(buff);
        }
        unlink(buff);
        print_errno();
      }
    }
  }
  if(rmdir(path) < 0) {
    print_errno();
    return 0;
  }
  print_fn_leave;
  return 1;
}
