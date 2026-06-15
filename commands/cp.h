#include "../lib.h"
#include "../nolibc/nolibc.h"
#include <sys/syscall.h>

static char COPY_BUFFER[COPY_BUFFER_SIZE];
static io_buffer_t COPY_IO = {COPY_BUFFER, COPY_BUFFER_SIZE, 0, -2};

static inline void read_file_with_read_write_to_file(int fd1, int fd2) {
  ssize_t ret;
  while ((ret = read(fd1, COPY_IO.buf + COPY_IO.pos, COPY_IO.size - COPY_IO.pos)) > 0) {
    COPY_IO.pos += ret;

    if (COPY_IO.pos >= COPY_IO.size) {
      write(fd2, COPY_IO.buf, COPY_IO.pos);
      COPY_IO.pos = 0;
    }
  }

  if (ret < 0) {
    print(&STDERR_IO, "Couldn't read file: ", _errno, _endl);
    exit(1);
  }

  if (COPY_IO.pos > 0) {
    write(fd2, COPY_IO.buf, COPY_IO.pos);
    COPY_IO.pos = 0;
  }
}

static inline void copy_file(int fd1, int fd2) {
  long ret;
  while ((ret = syscall(SYS_copy_file_range, fd1, NULL, fd2, NULL, INT_MAX, 0)) > 0) {
  }

  if (ret < 0 && (errno == EINVAL || errno == EXDEV)) {
    read_file_with_read_write_to_file(fd1, fd2);
    return;
  }

  if (ret < 0) {
    print(&STDERR_IO, "Couldn't copy file: ", _errno, _endl);
    exit(1);
  }
}

static inline void do_cp(int argc, char **argv) {
  if (argc < 3) {
    WRITE_LITERAL(STDERR_FILENO, "Using: cp SOURCE DESTINATION\n"
                                 "   or: cp SOURCE... DIRECTORY\n");
    exit(1);
  }

  if (argc == 3) {
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
      print(&STDERR_IO, "Couldn't open file ", argv[1], ": ", _errno, _endl);
      exit(1);
    }

    struct stat path_stat;
    if (stat(argv[2], &path_stat) != 0) {
      if (errno != ENOENT) {
        print(&STDERR_IO, "Couldn't open file ", argv[2], ": ", _errno, _endl);
        exit(1);
      }
    }

    int dst_fd = 0;
    if (S_ISDIR(path_stat.st_mode) || argv[2][strlen(argv[2]) - 1] == '/') {
      print(&COPY_IO, argv[2], "/", get_basename(argv[1]), "\0");
      dst_fd = open(COPY_IO.buf, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      COPY_IO.pos = 0;
    } else {
      dst_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    if (fd < 0) {
      print(&STDERR_IO, "Couldn't open file ", argv[2], ": ", _errno, _endl);
      exit(1);
    }

    copy_file(fd, dst_fd);
    close(dst_fd);
  } else {

    for (int i = 1; i < argc - 1; i++) {
      int fd = open(argv[i], O_RDONLY);
      if (fd < 0) {
        print(&STDERR_IO, "Couldn't open file ", argv[i], ": ", _errno, _endl);
        exit(1);
      }

      print(&COPY_IO, argv[argc - 1], "/", get_basename(argv[i]), "\0");
      int dst_fd = open(COPY_IO.buf, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      COPY_IO.pos = 0;

      copy_file(fd, dst_fd);
      close(dst_fd);

      if (i + 2 < argc)
        close(fd);
    }
  }

  exit(0);
}
