#include "fs_lib.h"

int main(void) {
  char* file = load_file_from_path("test.c", PROT_READ, MAP_PRIVATE);
  printf("%s\n", file);
  struct dirlist dirlist = {0};
  iterate_dir(&dirlist, ".");
  print_dlist(dirlist);
  create_dir_recursively("./hello/a/b");
  assert(path_exist("hello/a/b"));
  remove_dir_recursively("./hello");
  assert(!path_exist("hello"));
}
