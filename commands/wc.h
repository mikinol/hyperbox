#include "../mikinolibc/lib.h"

static inline long count_bytes_in_any_fd(int fd) {
  long readed = 0;

  long ret = 0;
  while ((ret = read(fd, STDIN_IO.buf, STDIN_IO.size)) > 0) {
    readed += ret;
  }

  if (ret < 0) {
    print(&STDERR_IO, "Cannot read fd: ", _errno, _endl);
    exit(1);
  }

  return readed;
}

static inline long count_newlines_in_fd(int fd) {
  long newlines = 0;

  long ret = 0;
  while ((ret = read(fd, STDIN_IO.buf, STDIN_IO.size)) > 0) {
    for (int i = 0; i < ret; i++) {
      if (STDIN_IO.buf[i] == '\n') {
        newlines++;
      }
    }
  }

  if (ret < 0) {
    print(&STDERR_IO, "Cannot read fd: ", _errno, _endl);
    exit(1);
  }

  return newlines;
}

noreturn static inline void do_wc(int argc, char **argv) {
  if (argc == 1) {
    long readed = count_bytes_in_any_fd(STDIN_FILENO);
    char *str = itoa(readed);
    write(STDOUT_FILENO, str, strlen(str));
    exit(0);
  }

  if (strcmp(argv[1], "-l") == 0) {
    if (argc == 2) {
      long newlines = count_newlines_in_fd(STDIN_FILENO);
      char *str = itoa(newlines);
      write(STDOUT_FILENO, str, strlen(str));
      exit(0);
    } else {
      long newlines = 0;
      int i = 2;
      while (true) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
          print(&STDERR_IO, "Cannot open file \'", argv[i], "\': ", _errno, _endl);
          exit(1);
        }

        newlines += count_newlines_in_fd(fd);

        i++;
        if (i == argc) {
          char *str = itoa(newlines);
          write(STDOUT_FILENO, str, strlen(str));
          exit(0);
        }

        close(fd);
      }
    }
  }

  long bytes = 0;
  int i = 1;
  bool is_skip_fd;
  while (true) {
    struct statx stx;
    int ret = syscall(__NR_statx, AT_FDCWD, argv[i], AT_STATX_DONT_SYNC, STATX_TYPE | STATX_SIZE, &stx);
    if (ret < 0) {
      print(&STDERR_IO, "Cannot stat file \'", argv[i], "\': ", _errno, _endl);
      exit(1);
    }

    if ((stx.stx_mask & STATX_TYPE) && S_ISREG(stx.stx_mode) && stx.stx_blocks != 0) {
      bytes += stx.stx_size;
      is_skip_fd = true;
      goto finish;
    }

    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      print(&STDERR_IO, "Cannot open file \'", argv[i], "\': ", _errno, _endl);
      exit(1);
    }

    bytes += count_bytes_in_any_fd(fd);

  finish:
    i++;
    if (i == argc) {
      char *str = itoa(bytes);
      write(STDOUT_FILENO, str, strlen(str));
      exit(0);
    }

    if (!is_skip_fd)
      close(fd);
  }

  exit(1);
}
