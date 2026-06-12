#include "../lib.h"
#include "../nolibc/nolibc.h"
#include <sys/syscall.h>

static char CAT_BUFFER[CAT_BUFFER_SIZE];
static unsigned long CAT_BUFFER_POS = 0;

static inline void read_file_with_read_write(int fd, char *filename) {
  ssize_t ret;
  while ((ret = read(fd, CAT_BUFFER + CAT_BUFFER_POS, CAT_BUFFER_SIZE - CAT_BUFFER_POS)) > 0) {
    CAT_BUFFER_POS += ret;

    if (CAT_BUFFER_POS >= (CAT_BUFFER_SIZE - CAT_BUFFER_KEEP)) {
      write(STDOUT_FILENO, CAT_BUFFER, CAT_BUFFER_POS);
      CAT_BUFFER_POS = 0;
    }
  }

  if (ret < 0) {
    errprint_literal("Couldn't read file ");
    errprint_string(filename);
    errprint_literal(" with status ");
    errprint_long(errno);
    errprint_flush();
    exit(1);
  }

  if (CAT_BUFFER_POS > 0) {
    write(STDOUT_FILENO, CAT_BUFFER, CAT_BUFFER_POS);
    CAT_BUFFER_POS = 0;
  }
}

static inline void read_files_to_stdout(int argc, char **argv) {
  int i = 1;
  while (true) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      errprint_literal("Couldn't open file ");
      errprint_string(argv[i]);
      errprint_literal(" with status ");
      errprint_long(fd);
      errprint_flush();
      exit(1);
    }

    if (strncmp(argv[i], "/proc/", 6) == 0 || strncmp(argv[i], "/sys/", 5) == 0 || strncmp(argv[i], "/dev/", 5) == 0) {
      read_file_with_read_write(fd, argv[i]);
      goto finish;
    }

    while (true) {
      ssize_t ret = syscall(SYS_sendfile, STDOUT_FILENO, fd, NULL, INT_MAX);

      if (ret > 0) {
        continue;
      }

      if (ret == 0) {
        break;
      }

      if (ret < 0) {
        if (errno == EINVAL) {
          read_file_with_read_write(fd, argv[i]);
          goto finish;
        }

        errprint_literal("Couldn't sendfile ");
        errprint_string(argv[i]);
        errprint_literal(" with status ");
        errprint_long(errno);
        errprint_flush();
        exit(1);
      }
    }

  finish:
    i++;
    if (i == argc)
      exit(0);

    close(fd);
  }
}

static inline void read_from_stdin() {
  long ret;
  while ((ret = read(STDIN_FILENO, CAT_BUFFER, sizeof(CAT_BUFFER))) > 0) {
    write(STDOUT_FILENO, CAT_BUFFER, ret);
  }

  if (ret < 0) {
    errprint_literal("Couldn't sendfile data from stdin to stdout with status ");
    errprint_long(ret);
    errprint_flush();
    exit(1);
  }
}

static inline void do_cat(int argc, char **argv) {
  if (argc == 1) {
    read_from_stdin();
  } else {
    read_files_to_stdout(argc, argv);
  }

  exit(0);
}
