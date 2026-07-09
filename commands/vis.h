#include "../mikinolibc/lib.h"

[[noreturn]] static inline void do_vis(int argc, char **argv) {
  ssize_t ret;
  while ((ret = read(STDIN_FILENO, STDIN_IO.buf, STDIN_IO.size)) > 0) {
    size_t last_normal = 0;
    for (size_t i = 0; i < ret; i++) {
      unsigned char c = STDIN_IO.buf[i];
      if (c == '\t') {
        print_array(&STDOUT_IO, STDIN_IO.buf + last_normal, i - last_normal);
        last_normal = i + 1;
        print(&STDOUT_IO, "^I");
      } else if (c == '\n') {
        print_array(&STDOUT_IO, STDIN_IO.buf + last_normal, i - last_normal);
        last_normal = i + 1;
        print(&STDOUT_IO, "$\n");
      } else if (c < 32) {
        print_array(&STDOUT_IO, STDIN_IO.buf + last_normal, i - last_normal);
        last_normal = i + 1;
        print_char(&STDOUT_IO, '^');
        print_char(&STDOUT_IO, (char)(c + '@'));
      } else if (c == 127) {
        print_array(&STDOUT_IO, STDIN_IO.buf + last_normal, i - last_normal);
        last_normal = i + 1;
        print(&STDOUT_IO, "^?");
      } else if (c > 127) {
        print_array(&STDOUT_IO, STDIN_IO.buf + last_normal, i - last_normal);
        last_normal = i + 1;
        char hex[5] = "\\x00";
        hex[2] = val_to_hex_lower(c >> 4);
        hex[3] = val_to_hex_lower(c & 0x0F);
        print_array(&STDOUT_IO, hex, sizeof(hex));
      }
    }

    print_array(&STDOUT_IO, STDIN_IO.buf + last_normal, ret - last_normal);
    print_flush(&STDOUT_IO);
  }

  if (unlikely(ret < 0)) {
    print(&STDERR_IO, "Cannot copy data from stdin to stdout: ", _errno, _endl);
    exit(1);
  }

  exit(0);
}
