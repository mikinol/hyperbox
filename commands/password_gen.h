#include "../mikinolibc/lib.h"

const char *dicts[2] = {
    "0123456789ABCDEF", // hexupper
    "0123456789abcdef", // hexlower
};

enum dictionaries { HEX_UPPER = 0, HEX_LOWER = 1, UNKNOWN = -1 };

static enum dictionaries parse_dict(const char *arg) {
  if (strcmp(arg, "hexupper") == 0)
    return HEX_UPPER;
  if (strcmp(arg, "hexlower") == 0)
    return HEX_LOWER;
  return UNKNOWN;
}

[[noreturn]] static inline void help() {
  WRITE_LITERAL(
      STDERR_FILENO,
      "Недостаточно агрументов, "
      "password_gen <длинна пароля> <количество паролей> (+s включает специальные символы) (-c выключает символы) (-n выключает цифры)\n"
      "Или можно password_gen <длинна пароля> <количество паролей> <список доступных символов>\n"
      "Если вместо количества паролей стоит \"n\" то это значит 1 пароль, без новой строки на конце"
      "И ещё можно использовать другие словари "
      "password_gen <длинна пароля> <количество паролей> @<название словаря>\n"
      "Словари:\n"
      "- hexupper — HEX с большими буквами\n"
      "- hexlower — HEX с маленькими буквами\n\n"
      "Или ещё можно генерировать uuid:\n"
      "password_gen uuid <количество uuid>\n"
      "Вместо количества uuid также можно указать n и получить 1 uuid без разделителя строки\n");
  exit(1);
}

bool is_end_without_endl = false;

static inline void parse_length_and_count(uint64_t *length, uint64_t *count, char **argv) {
  char *end;

  *length = strtoull(argv[0], &end, 10);

  if (unlikely(end == argv[0] || *end != '\0')) {
    WRITE_LITERAL(STDERR_FILENO, "Arguments parsing error: password length isn't a number\n");
    exit(1);
  }

  if (argv[1][0] == 'n' && argv[1][1] == '\0') {
    *count = 1;
    is_end_without_endl = true;
    return;
  }

  *count = strtoull(argv[1], &end, 10);

  if (unlikely(end == argv[1] || *end != '\0')) {
    WRITE_LITERAL(STDERR_FILENO, "Arguments parsing error: passwords count isn't a number\n");
    exit(1);
  }
}

static void parse_predefined_dictionary_from_argv(enum dictionaries *dict, int *pool_size, char *pool, char **argv) {
  *dict = parse_dict(argv[2] + 1);

  if (unlikely(*dict == UNKNOWN)) {
    print(&STDERR_IO, "Undefined dictionary: ", argv[2] + 1, _endl);
    exit(1);
  }

  *pool_size = strlen(dicts[*dict]);
  memcpy(pool, dicts[*dict], *pool_size);
}

static inline void parse_dictionary_from_argv(int *pool_size, char *pool, char **argv) {
  char *src = argv[2];
  *pool_size = strlen(src);

  if (unlikely(*pool_size > 256)) {
    WRITE_LITERAL(STDERR_FILENO, "Arguments parsing error: dictionary is larger than 256 symbols\n");
    exit(1);
  }

  memcpy(pool, src, *pool_size);
}

static inline void parse_contains_dictionary_from_argv(int *pool_size, char *pool, const int argc, char **argv) {
  static const char letters[52] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static const char numbers[10] = "0123456789";
  static const char symbols[25] = "!@#$%^&*()_+[]{}|;:',.<>?";

  bool contains_chars = true;
  bool contains_symbols = false;
  bool contains_numbers = true;

  for (int i = 2; i < argc; i++) {
    char *arg = argv[i];

    if (strlen(arg) != 2 || (arg[0] != '+' && arg[0] != '-')) {
      print(&STDERR_IO, "Ignored invalid flag: ", arg, _endl);
      continue;
    }

    bool state = (arg[0] == '+');

    switch (arg[1]) {
    case 'c':
      contains_chars = state;
      break;
    case 's':
      contains_symbols = state;
      break;
    case 'n':
      contains_numbers = state;
      break;
    default:
      print(&STDERR_IO, "Undefined flag type: ", arg[1], _endl);
    }
  }

  if (contains_chars) {
    memcpy(pool + *pool_size, letters, sizeof(letters));
    *pool_size += sizeof(letters);
  }
  if (contains_numbers) {
    memcpy(pool + *pool_size, numbers, sizeof(numbers));
    *pool_size += sizeof(numbers);
  }
  if (contains_symbols) {
    memcpy(pool + *pool_size, symbols, sizeof(symbols));
    *pool_size += sizeof(symbols);
  }
}

static inline int64_t recalc_bufsize(double success_percent, double needbytes) {
  double result = needbytes / success_percent;

  if (result > STDIN_IO.size) {
    return STDIN_IO.size;
  }

  if (success_percent != 1.0 && result < 16) {
    return 16;
  }

  int64_t truncated = result;
  return truncated + (truncated < result);
}

static inline void generate_uuids(int argc, char **argv) {
  static char default_uuid[37] = "xxxxxxxx-xxxx-4xxx-Vxxx-xxxxxxxxxxxx\n";

  long count;
  if (argv[1][0] == 'n' && argv[1][1] == '\0') {
    count = 1;
    is_end_without_endl = true;
  } else {
    char *end;
    count = strtoull(argv[1], &end, 10);

    if (unlikely(end == argv[1] || *end != '\0')) {
      WRITE_LITERAL(STDERR_FILENO, "Arguments parsing error: passwords count isn't a number\n");
      exit(1);
    }
  }

  int max_buf_size = count * 16;

  int current_char = 0;
  int current_password = 0;

  int bytes_read = 0;
  int current_byte = 0;
  while (true) {
    if (current_byte >= bytes_read) {
      current_byte = 0;

      int buf_size = max_buf_size - ((current_password * 15) + current_char);

      if (buf_size > STDIN_IO.size) {
        buf_size = STDIN_IO.size;
      }

      bytes_read = syscall(SYS_getrandom, STDIN_IO.buf, buf_size, 0);
      if (unlikely(bytes_read <= 0)) {
        print_flush(&STDOUT_IO);
        print(&STDERR_IO, "Getrandom failed: ", _errno, _endl);
        exit(1);
      }
    }

    unsigned char byte = STDIN_IO.buf[current_byte];
    char val1 = val_to_hex_lower(byte >> 4);
    char val2 = val_to_hex_lower(byte & 0x0F);

    if (current_char < 4) {
      default_uuid[current_char * 2] = val1;
      default_uuid[current_char * 2 + 1] = val2;
    } else if (current_char < 6) {
      default_uuid[current_char * 2 + 1] = val1;
      default_uuid[current_char * 2 + 2] = val2;
    } else if (current_char == 6) {
      default_uuid[15] = val1;
      default_uuid[16] = val2;
    } else if (current_char == 7) {
      default_uuid[17] = val1;
    } else if (current_char == 8) {
      default_uuid[19] = val_to_hex_lower((byte >> 6) | 0b1000);
      default_uuid[20] = val2;
    } else if (current_char == 9) {
      default_uuid[21] = val1;
      default_uuid[22] = val2;
    } else {
      default_uuid[current_char * 2 + 4] = val1;
      default_uuid[current_char * 2 + 5] = val2;

      if (current_char == 15) {
        print_array(&STDOUT_IO, default_uuid, sizeof(default_uuid) - is_end_without_endl);
        current_password++;
        current_char = -1;

        if (current_password == count)
          break;
      }
    }

    current_byte++;
    current_char++;
  }
}

static inline void run_do_password_gen(int argc, char **argv) {
  if (unlikely(argc < 2)) {
    help();
  }

  if (argv[0][0] == 'u' && argv[0][1] == 'u' && argv[0][2] == 'i' && argv[0][3] == 'd' && argv[0][4] == '\0') {
    generate_uuids(argc, argv);
    return;
  }

  uint64_t length, count;
  parse_length_and_count(&length, &count, argv);

  size_t password_size = length * count;

  char pool[256];
  int pool_size = 0;

  enum dictionaries dict = UNKNOWN;

  if (argc > 2 && *argv[2] == '@') {
    parse_predefined_dictionary_from_argv(&dict, &pool_size, pool, argv);
  } else if (argc > 2 && *argv[2] != '-' && *argv[2] != '+') {
    parse_dictionary_from_argv(&pool_size, pool, argv);
  } else {
    parse_contains_dictionary_from_argv(&pool_size, pool, argc, argv);
  }

  if (unlikely(pool_size == 0)) {
    WRITE_LITERAL(STDERR_FILENO, "Error: dictionary size = 0\n");
    exit(1);
  }

  const bool is_four = pool_size <= 16;
  int maxsize = (is_four ? 16 : 256);
  int limit = maxsize - (maxsize % pool_size);
  double success_percent = (double)limit / maxsize;

  int bytes_read = 0;
  int current_byte = 0;

  int current_write_byte = STDOUT_IO.pos;

  int current_password = 0;
  int current_password_char = 0;

  unsigned char byte;
  while (true) {
    if (current_byte >= bytes_read) {
      current_byte = 0;

      int64_t buf_size =
          recalc_bufsize(success_percent, (password_size - (current_password * length) - current_password_char) / (is_four ? 2.0 : 1.0));
      bytes_read = syscall(SYS_getrandom, STDIN_IO.buf, buf_size, 0);
      if (unlikely(bytes_read <= 0)) {
        print(&STDERR_IO, "Getrandom failed: ", _errno, _endl);
        exit(1);
      }
    }

    unsigned char current_read_byte = STDIN_IO.buf[current_byte];
    if (is_four) {
      byte = current_read_byte >> 4;
      if (byte < limit) {
        STDOUT_IO.buf[current_write_byte++] = pool[byte % pool_size];
        current_password_char++;

        if (current_password_char == length) {
          STDOUT_IO.buf[current_write_byte++] = '\n';
          current_password_char = 0;
          current_password++;
        }

        if (current_write_byte > STDOUT_IO.size - 3) {
          write(1, STDOUT_IO.buf, current_write_byte);
          current_write_byte = 0;
        }

        if (current_password == count) {
          break;
        }
      }

      byte = current_read_byte & 0x0F;
      current_byte++;
      if (byte < limit) {
        STDOUT_IO.buf[current_write_byte++] = pool[byte % pool_size];
        current_password_char++;

        if (current_password_char == length) {
          STDOUT_IO.buf[current_write_byte++] = '\n';
          current_password_char = 0;
          current_password++;
        }

        if (current_write_byte > STDOUT_IO.size - 3) {
          write(1, STDOUT_IO.buf, current_write_byte);
          current_write_byte = 0;
        }
      }
    } else {
      byte = current_read_byte;
      current_byte++;

      if (byte >= limit)
        continue;

      STDOUT_IO.buf[current_write_byte++] = pool[byte % pool_size];
      current_password_char++;

      if (current_password_char == length) {
        STDOUT_IO.buf[current_write_byte++] = '\n';
        current_password_char = 0;
        current_password++;
      }

      if (current_write_byte > STDOUT_IO.size - 3) {
        write(1, STDOUT_IO.buf, current_write_byte);
        current_write_byte = 0;
      }
    }

    if (current_password == count) {
      break;
    }
  }

  if (current_write_byte > 0)
    STDOUT_IO.pos = current_write_byte;
}

[[noreturn]] void do_password_gen(int argc, char **argv) {
  if (unlikely(argc < 3)) {
    help();
  }

  int last_i = 1;
  for (int i = 1;;) {
    if ((argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == '\0')) {
      run_do_password_gen(i - last_i, argv + last_i);
      last_i = i + 1;
    }

    if (++i == argc) {
      run_do_password_gen(i - last_i, argv + last_i);
      break;
    }
  }

  print_flush(&STDOUT_IO);
  exit(0);
}
