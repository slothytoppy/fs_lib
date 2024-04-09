#include "../nom/nom.h"

int main(int argc, char* argv[]) {
  char* args[] = {__FILE__, NULL};
  rebuild(1, args, __FILE__, "gcc");
  Nom_cmd cmd = {0};
  if(argc >= 2 && strcmp(argv[1], "-d") == 0) {
    nom_cmd_append_many(&cmd, 6, "gcc", "-g", "-DDEBUG", "test.c", "-o", "test");
  } else {
    nom_cmd_append_many(&cmd, 5, "gcc", "-g", "test.c", "-o", "test");
  }
  nom_run_sync(cmd);
  nom_cmd_reset(&cmd);
  nom_cmd_append(&cmd, "./test");
  nom_run_path(cmd, NULL);
}
