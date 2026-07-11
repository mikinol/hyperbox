#include "../mikinolibc/lib.h"

noreturn static inline void do_urlencode(int argc, char **argv) {
  long ret;
  while ((ret = read(STDIN_FILENO, STDIN_IO.buf, STDIN_IO.size)) > 0) {
    char *last_normal = STDIN_IO.buf;
    for (char *i = STDIN_IO.buf; i < (STDIN_IO.buf + ret); i++) {
      if ((*i >= 'A' && *i <= 'Z') || (*i >= 'a' && *i <= 'z') || (*i >= '0' && *i <= '9') || *i == '-' || *i == '_' || *i == '.' ||
          *i == '~' || *i == ':' || *i == '/' || *i == '?' || *i == '#' || *i == '[' || *i == ']' || *i == '@' || *i == '!' || *i == '$' ||
          *i == '&' || *i == '\'' || *i == '(' || *i == ')' || *i == '*' || *i == '+' || *i == ',' || *i == ';' || *i == '=' ||
          *i == '\n') {
        continue;
      }

      if (last_normal != i) {
        print_array(&STDOUT_IO, last_normal, i - last_normal);
      }

      print(&STDOUT_IO, "%", val_to_hex(((unsigned char)*i) >> 4), val_to_hex(((unsigned char)*i) % 16));
      last_normal = i + 1;
    }

    if (last_normal < STDIN_IO.buf + ret) {
      print_array(&STDOUT_IO, last_normal, (STDIN_IO.buf + ret) - last_normal);
    }
  }

  print_flush(&STDOUT_IO);

  if (unlikely(ret < 0)) {
    print(&STDERR_IO, "Cannot read stdin: ", _errno, _endl);
    exit(1);
  }

  exit(0);
}

noreturn static inline void do_urldecode(int argc, char **argv) {
  bool is_broken;

  long ret;
  while ((ret = read(STDIN_FILENO, STDIN_IO.buf, STDIN_IO.size)) > 0) {
    char *last_normal = STDIN_IO.buf;
    char *end = STDIN_IO.buf + ret;
    for (char *i = STDIN_IO.buf; i < end; i++) {
      if (*i == '%') {
        if (last_normal != i) {
          print_array(&STDOUT_IO, last_normal, i - last_normal);
        }

        if (unlikely(i + 2 >= end)) {
          is_broken = true;
          continue;
        }

        i++;
        char ch = hex_to_val(*i);
        if (unlikely(ch == -1)) {
          is_broken = true;
          continue;
        }

        i++;
        char ch1 = hex_to_val(*i);
        if (unlikely(ch1 == -1)) {
          is_broken = true;
          continue;
        }
        ch = (ch << 4) | ch1;

        print_array(&STDOUT_IO, &ch, 1);
        last_normal = i + 1;
      }
    }

    if (last_normal < end) {
      print_array(&STDOUT_IO, last_normal, end - last_normal);
    }
  }

  print_flush(&STDOUT_IO);

  if (unlikely(ret < 0)) {
    print(&STDERR_IO, "Cannot read stdin: ", _errno, _endl);
    exit(1);
  }

  if (is_broken)
    print(&STDERR_IO, "Broken string", _endl);

  exit(is_broken);
}
