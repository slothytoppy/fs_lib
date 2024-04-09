#include "./fs_lib.h"

int main(void) {
  struct dirlist dirlist = {0};
  dlist_append(&dirlist, "aaa");
  dlist_append(&dirlist, "AAA");
  print_dlist(dirlist);
  create_dir_recursively("hello/a/b/test.c");
  remove_dir_recursively("hello/a/b/test.c");
}
