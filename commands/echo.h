#include "../lib.h"
#include "../nolibc/nolibc.h"

static inline void do_echo(int argc, char **argv) {
  if (argc == 1) {
    WRITE_LITERAL(STDOUT_FILENO, "\n");
    exit(0);
  }

  bool is_not_do_newline = strcmp(argv[1], "-n") == 0;
  if (is_not_do_newline && argc == 2) {
    exit(0);
  }

  char *start_pointer = argv[1 + is_not_do_newline];
  size_t current_reading = 2 + is_not_do_newline;
  size_t i = 0;
  while (true) {
    if (start_pointer[i] == '\0') {
      if (current_reading < argc) {
        start_pointer[i] = ' ';
        current_reading++;
      } else if (is_not_do_newline) {
        write(STDOUT_FILENO, start_pointer, i);
        exit(0);
      } else {
        start_pointer[i] = '\n';
        write(STDOUT_FILENO, start_pointer, i + 1);
        exit(0);
      }
    }

    i++;
  }

  exit(0);
}
