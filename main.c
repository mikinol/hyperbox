#include "nolibc/nolibc.h"
#include <sys/syscall.h>

#include "lib.h"

#include "./commands/cat.h"

int main(int argc, char **argv) {
  if (argc < 1)
    exit(1);

  const char *cmd = get_basename(argv[0]);
  if (strcmp(cmd, "cat") == 0) {
    do_cat(argc, argv);
  }

  exit(1);
}
