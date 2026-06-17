#include "../lib.h"

static inline void do_recursive_mkdir(char *path) {
  bool was_slash = false;

  for (int j = 1; path[j] != '\0'; j++) {
    if (path[j] == '/') {
      if (was_slash)
        continue;

      path[j] = '\0';

      int ret = mkdir(path, 0777);
      if (ret < 0 && errno != EEXIST) {
        print(&STDERR_IO, "Cannot create directory \'", path, "\': ", _errno, _endl);
        exit(1);
      }

      path[j] = '/';
      was_slash = true;
      continue;
    }

    was_slash = false;
  }

  if (was_slash == false) {
    int ret = mkdir(path, 0777);
    if (ret < 0 && errno != EEXIST) {
      print(&STDERR_IO, "Cannot create directory \'", path, "\': ", _errno, _endl);
      exit(1);
    }
  }
}

[[noreturn]] static inline void do_mkdir(int argc, char **argv) {
  if (argc == 1) {
    WRITE_LITERAL(STDERR_FILENO, "Usage: mkdir <directory...>\n"
                                 "   or: mkdir -p <directory...>\n");
    exit(1);
  }

  if (strcmp(argv[1], "-p") == 0) {
    for (int i = 2; i < argc; i++) {
      do_recursive_mkdir(argv[i]);
    }
  } else {
    for (int i = 1; i < argc; i++) {
      int ret = mkdir(argv[i], 0777);

      if (ret < 0) {
        print(&STDERR_IO, "Couldn't mkdir ", argv[i], ": ", _errno, _endl);
        exit(1);
      }
    }
  }

  exit(0);
}
