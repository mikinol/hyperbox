#include "../mikinolibc/lib.h"

[[noreturn]] static inline void do_tee(int argc, char **argv) {
  int fds[argc];
  fds[0] = STDOUT_FILENO;
  for (int i = 1; i < argc; i++) {
    fds[i] = open(argv[i], O_WRONLY | O_CREAT | O_TRUNC);
    if (fds[i] < 0) {
      print(&STDERR_IO, "Cannot open file ", argv[i], ": ", _errno, _endl);
      exit(1);
    }
  }

  long ret;
  while ((ret = read(STDIN_FILENO, STDIN_IO.buf, STDIN_IO.size)) > 0) {
    for (int i = 0; i < argc; i++) {
      write(fds[i], STDIN_IO.buf, ret);
    }
  }

  if (ret < 0) {
    print(&STDERR_IO, "Cannot read stdin: ", _errno, _endl);
    exit(1);
  }

  for (int i = 0; i < argc; i++) {
    close(fds[i]);
  }

  exit(0);
}
