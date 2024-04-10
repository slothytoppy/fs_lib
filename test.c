#include "fs_lib.h"

int main(void) {
  char* file = load_file_from_path("test.c", PROT_READ, MAP_PRIVATE);
  printf("%s\n", file);
  return 0;
  struct dirlist dirlist = {0};
  iterate_dir(&dirlist, ".");
  print_dlist(dirlist);
  create_dir_recursively("./hello/a/b");
  assert(does_path_exist("hello/a/b"));
  remove_dir_recursively("./hello");
  assert(!does_path_exist("hello"));
}
