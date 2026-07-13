#include "../mikinolibc/lib.h"

static inline void read_files_to_stdout(int argc, char **argv) {
  int i = 1;
  while (true) {
    int fd = open(argv[i], O_RDONLY);
    if (unlikely(fd < 0)) {
      print(&STDERR_IO, "Cannot open file ", argv[i], ": ", _errno, _endl);
      exit(1);
    }

    if (strncmp(argv[i], "/proc/", 6) == 0 || strncmp(argv[i], "/sys/", 5) == 0 || strncmp(argv[i], "/dev/", 5) == 0) {
      goto read_with_read_write;
    }

    print_flush(&STDOUT_IO);

    ssize_t ret;
    while ((ret = syscall(SYS_sendfile, STDOUT_FILENO, fd, NULL, INT_MAX)) > 0) {
    }

    if (ret < 0) {
      if (unlikely(errno != EINVAL)) {
        print(&STDERR_IO, "Couldn't sendfile ", argv[i], ": ", _errno, _endl);
        exit(1);
      }

    read_with_read_write:
      long ret = print_fd_to_end(&STDOUT_IO, fd);
      if (unlikely(ret < 0)) {
        print(&STDERR_IO, "Cannot read file ", argv[i], ": ", _errno, _endl);
        exit(1);
      }
    }

    if (++i >= argc)
      return;

    close(fd);
  }
}

noreturn static inline void do_cat(int argc, char **argv) {
  if (argc == 1) {
    long ret = print_fd_to_end(&STDOUT_IO, STDIN_FILENO);

    if (unlikely(ret < 0)) {
      print(&STDERR_IO, "Cannot copy data from stdin to stdout: ", _errno, _endl);
      exit(1);
    }
  } else {
    read_files_to_stdout(argc, argv);
  }

  print_flush(&STDOUT_IO);
  exit(0);
}
