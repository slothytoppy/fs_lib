#include "fs_lib.h"

int main(void) {
  create_dir_recursively("./hello/a/b");
  assert(does_path_exist("hello/a/b"));
  remove_dir_recursively("./hello");
  assert(!does_path_exist("hello"));
}
