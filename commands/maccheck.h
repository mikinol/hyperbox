#include "../lib.h"

#ifndef OUI_PATH
#define OUI_PATH "/usr/share/hwdata/oui.txt"
#endif

#ifdef OUI_SIZE
static long oui_file_size = OUI_SIZE;
#else
static long oui_file_size;
#endif

static char *oui_file;

static inline void open_ouifile() {
  int fd = open(OUI_PATH, O_RDONLY);
  if (unlikely(fd == -1)) {
    print(&STDERR_IO, "Cannot open oui file ", OUI_PATH, ": ", _errno, _endl);
    exit(1);
  }

#ifndef OUI_SIZE
  struct stat sb;
  if (unlikely(fstat(fd, &sb) == -1)) {
    print(&STDERR_IO, "Cannot get oui file size: ", _errno, _endl);
    exit(1);
  }

  oui_file_size = sb.st_size;
#endif

  oui_file = mmap(NULL, oui_file_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);

  if (unlikely(oui_file == MAP_FAILED)) {
    print(&STDERR_IO, "Cannot mmap oui file: ", _errno, _endl);
    exit(1);
  }

  close(fd);
}

static inline uint32_t simple_oui_to_int(char *start) {
  uint32_t res = 0;
  for (int i = 0; i < 6; i++) {
    res = (res << 4) | hex_to_val(start[i]);
  }

  return res;
}

void print_oui_base16(uint32_t res) {
  uint8_t b1 = (uint8_t)((res >> 16) & 0xFF);
  uint8_t b2 = (uint8_t)((res >> 8) & 0xFF);
  uint8_t b3 = (uint8_t)(res & 0xFF);

  char buf[6];

  buf[0] = val_to_hex(b1 >> 4);
  buf[1] = val_to_hex(b1 & 0x0F);

  buf[2] = val_to_hex(b2 >> 4);
  buf[3] = val_to_hex(b2 & 0x0F);

  buf[4] = val_to_hex(b3 >> 4);
  buf[5] = val_to_hex(b3 & 0x0F);

  print_array(&STDOUT_IO, buf, 6);
}

static int count_args = 0;
static inline void parse_argv_to_ints(int argc, char **argv) {
  if (unlikely(argc - 1 > STDIN_IO.size / sizeof(uint32_t))) {
    print(&STDERR_IO, "Input buffer is too small to fit ", argc - 1,
          " elements, maximum buffer size is: ", STDIN_IO.size / sizeof(uint32_t));
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    uint32_t res = 0;
    size_t count = 0;

    for (size_t j = 0; argv[i][j] != '\0'; j++) {
      char c = argv[i][j];
      if (c == ':' || c == '-') {
        continue;
      }

      int val = hex_to_val(c);
      if (val == -1) {
        print(&STDERR_IO, "Warning: ignoring non-hex symbol: ", c, _endl);
        continue;
      }

      if (count == 6)
        break;

      res = (res << 4) | val;
      count++;
    }

    if (count < 6) {
      print(&STDERR_IO, "Warning: cannot parse OUI: ", argv[i], ", skipped", _endl);
      continue;
    }

    uint8_t first_byte = (uint8_t)((res >> 16) & 0xFF);

    if (res == 0x00FFFFFF) {
      print(&STDOUT_IO, "OUI: FFFFFF\nType: Broadcast\n\n");
      continue;
    } else {
      if (first_byte & 0x01) {
        print(&STDOUT_IO, "OUI: ");
        print_oui_base16(res);
        print(&STDOUT_IO, "\nType: Multicast\n\n");
        continue;
      }

      if (first_byte & 0x02) {
        print(&STDOUT_IO, "OUI: ");
        print_oui_base16(res);
        print(&STDOUT_IO, "\nType: Local generated\n\n");
        continue;
      }
    }

    ((uint32_t *)STDIN_IO.buf)[count_args] = res;
    count_args++;
  }
}

[[noreturn]] static inline void do_maccheck(int argc, char **argv) {
  if (argc < 2) {
    WRITE_LITERAL(STDERR_FILENO, "Using: maccheck OUI/MAC...\n");
    exit(1);
  }

  parse_argv_to_ints(argc, argv);
  if (count_args == 0) {
    print(&STDOUT_IO, _flush);
    exit(0);
  }

  open_ouifile();

  int8_t num = -4;
  int count_printed = 0;
  long old_endl_pos = 0;
  bool is_printing = false;
  for (long i = 0; i < oui_file_size && count_printed < count_args; i++) {
    if (oui_file[i] == '\n') {
      if (old_endl_pos + 1 == i) {
        num = 0;
        continue;
      }

      if (num == 1) {
        char *readpos = oui_file + old_endl_pos + 1;
        uint32_t cur_oui = simple_oui_to_int(readpos);
        for (int j = 0; j < count_args; j++) {
          if (cur_oui == ((uint32_t *)STDIN_IO.buf)[j]) {
            print(&STDOUT_IO, "OUI: ");
            print_array(&STDOUT_IO, readpos, 6);
            print(&STDOUT_IO, "\nType: Global\nVendor: ");
            readpos += 22;
            print_array(&STDOUT_IO, readpos, oui_file + i - readpos + 1);

            is_printing = true;
            break;
          }
        }
      } else if (is_printing) {
        char *readpos = oui_file + old_endl_pos + 5;
        if (num == 2) {
          print(&STDOUT_IO, "Address: ");
          print_array(&STDOUT_IO, readpos, oui_file + i - readpos);
          print(&STDOUT_IO, ", ");
        } else if (num == 3) {
          print_array(&STDOUT_IO, readpos, oui_file + i - readpos + 1);
        } else if (num == 4) {
          print(&STDOUT_IO, "Country: ");
          print_array(&STDOUT_IO, readpos, oui_file + i - readpos + 1);
          print(&STDOUT_IO, "\n");

          i++;
          num = -1;
          count_printed++;
          is_printing = false;
        }
      } else if (num == 4) {
        i++;
        num = -1;
      }

      num++;
      old_endl_pos = i;
    }
  }

  print(&STDOUT_IO, _flush);
  exit(0);
}
