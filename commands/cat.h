#include "../lib.h"

static inline void read_file_with_read_write(int fd, char *filename) {
  long ret = print_fd_to_end(&STDOUT_IO, fd);
  if (ret < 0) {
    print(&STDERR_IO, "Cannot read file ", filename, ": ", _errno, _endl);
    exit(1);
  }
}

static inline void read_files_to_stdout(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      print(&STDERR_IO, "Cannot open file ", argv[i], ": ", _errno, _endl);
      exit(1);
    }

    if (strncmp(argv[i], "/proc/", 6) == 0 || strncmp(argv[i], "/sys/", 5) == 0 || strncmp(argv[i], "/dev/", 5) == 0) {
      read_file_with_read_write(fd, argv[i]);
      goto finish;
    }

    print_flush(&STDOUT_IO);
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
        print(&STDERR_IO, "Couldn't sendfile ", argv[i], ": ", _errno, _endl);
        exit(1);
      }
    }

  finish:
    if (i == argc - 1)
      return;

    close(fd);
  }
}

[[noreturn]] static inline void do_cat(int argc, char **argv) {
  if (argc == 1) {
    long ret = print_fd_to_end(&STDOUT_IO, STDIN_FILENO);

    if (ret < 0) {
      print(&STDERR_IO, "Cannot copy data from stdin to stdout: ", _errno, _endl);
      exit(1);
    }

    print_flush(&STDOUT_IO);
  } else {
    read_files_to_stdout(argc, argv);
    print_flush(&STDOUT_IO);
  }

  exit(0);
}
