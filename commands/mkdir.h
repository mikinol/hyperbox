#include "../lib.h"
#include "../nolibc/nolibc.h"

static inline void do_recursive_mkdir(int argc, char **argv) {
  for (int i = 2; i < argc; i++) {
    char *path = argv[i];
    for (int j = 1; path[j] != '\0'; j++) {
      if (path[j] == '/') {
        path[j] = '\0';

        int ret = mkdir(path, 0777);
        if (ret < 0 && errno != EEXIST) {
          errprint_literal("Couldn't mkdir ");
          errprint_string(argv[i]);
          errprint_literal(" with status ");
          errprint_long(errno);
          errprint_flush();
          exit(1);
        }

        path[j] = '/';
      }
    }
  }
}

static inline void do_default_mkdir(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    int ret = mkdir(argv[i], 0777);

    if (ret < 0) {
      errprint_literal("Couldn't mkdir ");
      errprint_string(argv[i]);
      errprint_literal(" with status ");
      errprint_long(errno);
      errprint_flush();
      exit(1);
    }
  }
  exit(0);
}

static inline void do_mkdir(int argc, char **argv) {
  if (argc == 1) {
    WRITE_LITERAL(STDERR_FILENO, "Usage: mkdir <directory...>\n"
                                 "       mkdir -p <directory...>");
    exit(1);
  }

  if (strcmp(argv[1], "-p") == 0) {
    do_recursive_mkdir(argc, argv);
  } else {
    do_default_mkdir(argc, argv);
  }

  exit(0);
}
