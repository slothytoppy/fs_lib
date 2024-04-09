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

void print_errno(void) {
  printf("%s\n", strerror(errno));
  return;
}

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

struct dirlist {
  char** dir;
  unsigned count;
  unsigned capacity;
};

int dlist_append(struct dirlist* dirlist, char* item) {
  if(dirlist->count == 0) {
    dirlist->capacity = 256;
  } else {
    dirlist->dir = (char**)realloc(dirlist->dir, (dirlist->count + 1) * sizeof(dirlist->dir));
    if(dirlist->count >= dirlist->capacity) {
      dirlist->capacity *= 2;
      dirlist->dir = (char**)realloc(dirlist->dir, dirlist->capacity * sizeof(dirlist->dir));
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

struct dirlist iterate_dir(char* path) {
  struct dirlist dirlist = {0};
  if(!does_path_exist(path)) {
    debug_print("%s does not exist\n", path);
    return dirlist;
  }
  if(!is_path_dir(path)) {
    debug_print("%s is not a directory\n", path);
    return dirlist;
  }
  DIR* dir = opendir(path);
  struct dirent* dirent;
  int path_len = strlen(path);
  char** path_list = (char**)calloc(1, path_len * sizeof(char*));
  for(int i = 0; (dirent = readdir(dir)) != NULL; i++) {
    if(strlen(dirent->d_name) <= 2 && dirent->d_name[0] == '.' || dirent->d_name[1] == '.') {
      continue;
    }
    if(strlen(dirent->d_name) > strlen(path)) {
      path_len += strlen(dirent->d_name);
      path_list = (char**)realloc(path_list, path_len * sizeof(char*));
    }
    path_list[i] = dirent->d_name;
    printf("path_list[%d]=%s\n", i, path_list[i]);
  }
  return dirlist;
}

int create_file(char* path) {
  if(!does_path_exist(path)) {
    if(creat(path, 0644) < 0) {
      print_errno();
      return 0;
    }
    debug_print("created %s\n", path);
    return 1;
  }
  debug_print("%s already exists\n", path);
  return 0;
}

int delete_file(char* path) {
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

int create_dir(char* path) {
  if(!does_path_exist(path)) {
    if(mkdir(path, 0755) < 0) {
      print_errno();
      return 0;
    }
    debug_print("created %s\n", path);
    return 1;
  }
  debug_print("%s already exists\n", path);
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

void* load_file_from_path(char* path) {
  if(!does_path_exist(path)) {
    debug_print("%s does not exist\n", path);
    return 0;
  }
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
  void* file_mem;
  file_mem = mmap(0, fi.st_size, PROT_READ, MAP_PRIVATE, fd_path, 0);
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
  char* file = (char*)calloc(1, file_len);
  int i = 0;
  if(path[0] == '.' && path[1] == '/') {
    i = 2;
  }
  for(; i <= file_len; i++) {
    file[i] = path[i];
    if(file[i] == '/') {
      printf("%d %c\n", i, file[i]);
      if(does_path_exist(file)) {
        continue;
      }
      create_dir(file);
    }
    printf("file=%s\n", file);
  }
  create_file(file);
  return 1;
}

int create_dir_recursively(char* path) {
  if(does_path_exist(path)) {
    return 1;
  }
  int file_len = strlen(path);
  char* file = (char*)calloc(1, file_len);
  int i = 0;
  if(path[0] == '.' && path[1] == '/') {
    i = 2;
  }
  for(; i <= file_len; i++) {
    file[i] = path[i];
    if(file[i] == '/') {
      debug_print("%d %c\n", i, file[i]);
      if(does_path_exist(file)) {
        continue;
      }
      create_dir(file);
    }
    // debug_print("file=%s\n", file);
  }
  create_dir(file);
  return 1;
}

int remove_dir_recursively(char* path) {
  if(!does_path_exist(path)) {
    printf("%s does not exist\n");
    return 0;
  }
  unsigned plen = strlen(path);
  char* buff = (char*)calloc(1, plen);
  for(int i = 0; i < plen; i++) {
    buff[i] = path[i];
    if(path[i] == '/') {
      struct dirlist dirlist = iterate_dir(buff);
      for(int i = 0; i < dirlist.count; i++) {
        printf("dirlist.dir[%d]=%s\n", i, dirlist.dir);
      }
    }
  }
  return 1;
}
