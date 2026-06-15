#include "nolibc/nolibc.h"
#include <sys/syscall.h>

#include "lib.h"

#include "./commands/cat.h"
#include "./commands/cp.h"
#include "./commands/echo.h"
#include "./commands/mkdir.h"
#include "./commands/wc.h"

int main(int argc, char **argv) {
  if (argc < 1)
    exit(1);

  const char *cmd = get_basename(argv[0]);
  if (strcmp(cmd, "cat") == 0) {
    do_cat(argc, argv);
  } else if (strcmp(cmd, "mkdir") == 0) {
    do_mkdir(argc, argv);
  } else if (strcmp(cmd, "echo") == 0) {
    do_echo(argc, argv);
  } else if (strcmp(cmd, "wc") == 0) {
    do_wc(argc, argv);
  } else if (strcmp(cmd, "cp") == 0) {
    do_cp(argc, argv);
  }

  WRITE_LITERAL(STDERR_FILENO, "Incorrect cmd");
  exit(1);
}
