#include "./fs_lib.h"

int main(void) {
  create_dir_recursively("hello/a/b/test.c");
  remove_dir_recursively("hello/a/b/test.c");
}
