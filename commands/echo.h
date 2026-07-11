#include "../mikinolibc/lib.h"

noreturn static inline void do_echo(int argc, char **argv) {
  if (unlikely(argc == 1)) {
    WRITE_LITERAL(STDOUT_FILENO, "\n");
    goto exit;
  }

  bool is_not_do_newline = argv[1][0] == '-' && argv[1][1] == 'n' && argv[1][2] == '\0';
  if (unlikely(is_not_do_newline && argc == 2)) {
    goto exit;
  }

  size_t start_i = 1 + is_not_do_newline;
  for (size_t i = start_i; i < argc - 1; i++) {
    *(argv[i + 1] - 1) = ' ';
  }

  size_t last_argv_strlen = strlen(argv[argc - 1]);
  if (!is_not_do_newline) {
    argv[argc - 1][last_argv_strlen++] = '\n';
  }

  write(STDOUT_FILENO, argv[start_i], argv[argc - 1] + last_argv_strlen - argv[start_i]);

exit:
  exit(0);
}
