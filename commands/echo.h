#include "../lib.h"

static inline void do_echo(int argc, char **argv) {
  if (argc == 1) {
    WRITE_LITERAL(STDOUT_FILENO, "\n");
    exit(0);
  }

  bool is_not_do_newline = strcmp(argv[1], "-n") == 0;
  if (is_not_do_newline && argc == 2) {
    exit(0);
  }

  size_t start_i = 2 + is_not_do_newline;
  for (size_t i = start_i; i < argc - 1; i++) {
    *(argv[i + 1] - 1) = ' ';
  }

  size_t last_argv_strlen = strlen(argv[argc - 1]);
  if (is_not_do_newline) {
    write(STDOUT_FILENO, argv[start_i], argv[argc - 1] + last_argv_strlen - argv[start_i]);
    exit(0);
  }

  argv[argc - 1][last_argv_strlen] = '\n';
  write(STDOUT_FILENO, argv[start_i], argv[argc - 1] + last_argv_strlen + 1 - argv[start_i]);
  exit(0);
}
