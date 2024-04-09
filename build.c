#include "../nom/nom.h"

int main(void) {
  char* argv[] = {__FILE__, NULL};
  rebuild(1, argv, __FILE__, "gcc");
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 5, "gcc", "-g", "test.c", "-o", "test");
  nom_run_sync(cmd);
}
