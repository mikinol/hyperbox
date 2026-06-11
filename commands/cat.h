#include "../lib.h"
#include "../nolibc/nolibc.h"
#include <sys/syscall.h>

static char CAT_BUFFER[CAT_BUFFER_SIZE];
static unsigned long CAT_BUFFER_POS = 0;

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

    struct statx stx;
    int status = statx(fd, NULL, AT_EMPTY_PATH, STATX_SIZE, &stx);
    if (status != 0) {
      errprint_literal("Couldn't statx file ");
      errprint_string(argv[i]);
      errprint_literal(" with status ");
      errprint_long(status);
      errprint_flush();
      exit(1);
    }
    if (!(stx.stx_mask & STATX_SIZE)) {
      errprint_literal("Incorrect statx mask ");
      errprint_long(stx.stx_mask);
      errprint_literal(" with file ");
      errprint_string(argv[i]);
      errprint_flush();
      exit(1);
    }
    if (stx.stx_size == 0) {
      long ret;
      while ((ret = read(fd, CAT_BUFFER + CAT_BUFFER_POS, CAT_BUFFER_SIZE - CAT_BUFFER_POS)) > 0) {
        CAT_BUFFER_POS += ret;

        if (CAT_BUFFER_POS >= (CAT_BUFFER_SIZE - CAT_BUFFER_KEEP)) {
          write(STDOUT_FILENO, CAT_BUFFER, CAT_BUFFER_POS);
          CAT_BUFFER_POS = 0;
        }
      }

      if (ret < 0) {
        errprint_literal("Couldn't read file ");
        errprint_string(argv[i]);
        errprint_literal(" with status ");
        errprint_long(ret);
        errprint_flush();
        exit(1);
      }

      goto finish;
    }

    ssize_t readed = 0;
    while (readed != stx.stx_size) {
      if (CAT_BUFFER_POS > 0) {
        write(STDOUT_FILENO, CAT_BUFFER, CAT_BUFFER_POS);
        CAT_BUFFER_POS = 0;
      }

      ssize_t retuned = syscall(SYS_sendfile, STDOUT_FILENO, fd, NULL, stx.stx_size - readed);
      if (retuned == 0) {
        break;
      } else if (retuned < 0) {
        errprint_literal("Couldn't sendfile ");
        errprint_string(argv[i]);
        errprint_literal(" with status ");
        errprint_long(retuned);
        errprint_flush();
        exit(1);
      }

      readed += retuned;
    }

  finish:
    i++;
    if (i == argc) {
      if (CAT_BUFFER_POS > 0)
        write(STDOUT_FILENO, CAT_BUFFER, CAT_BUFFER_POS);

      exit(0);
    }

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
