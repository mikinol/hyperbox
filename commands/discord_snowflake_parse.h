#include "../mikinolibc/lib.h"

#define DISCORD_EPOCH 1420070400000ULL

struct my_tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
};

static const int days_in_months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void seconds_to_utc(long long seconds, struct my_tm *result) {
  result->tm_sec = seconds % 60;
  long long minutes = seconds / 60;

  result->tm_min = minutes % 60;
  long long hours = minutes / 60;

  result->tm_hour = hours % 24;
  long long days = hours / 24;

  int year = 1970;
  while (1) {
    int is_leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    int days_in_year = is_leap ? 366 : 365;

    if (days >= days_in_year) {
      days -= days_in_year;
      year++;
    } else {
      break;
    }
  }
  result->tm_year = year - 1900;

  int month = 0;
  for (month = 0; month < 12; month++) {
    int dim = days_in_months[month];
    if (month == 1 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
      dim = 29;
    }

    if (days >= dim) {
      days -= dim;
    } else {
      break;
    }
  }
  result->tm_mon = month;

  result->tm_mday = days + 1;
}

char *itoa_padded(int value, int width) {
  char *str = itoa(value);
  int len = strlen(str);

  if (len < width) {
    int padding = width - len;

    memmove(str + padding, str, len + 1);

    for (int i = 0; i < padding; i++) {
      str[i] = '0';
    }
  }

  return str;
}

noreturn void do_discord_snowflake_parse(int argc, char *argv[]) {
  if (argc < 2) {
    print(&STDERR_IO, "Using: ", argv[0], " <snowflake_id>", _endl);
    exit(1);
  }

  uint64_t snowflake = strtoull(argv[1], NULL, 10);
  if (snowflake == 0) {
    WRITE_LITERAL(STDERR_FILENO, "Parse error: incorrect ID.\n");
    exit(1);
  }

  uint64_t timestamp = (snowflake >> 22) + DISCORD_EPOCH;
  uint8_t worker_id = (snowflake >> 17) & 0x1F;
  uint8_t process_id = (snowflake >> 12) & 0x1F;
  uint16_t increment = snowflake & 0xFFF;

  time_t seconds = timestamp / 1000;
  int milliseconds = timestamp % 1000;

  struct my_tm utc_time;

  seconds_to_utc(seconds, &utc_time);
  print(&STDOUT_IO, "Дата (UTC): ", utc_time.tm_year + 1900, "-", itoa_padded(utc_time.tm_mon + 1, 2), "-",
        itoa_padded(utc_time.tm_mday, 2));
  print(&STDOUT_IO, " ", itoa_padded(utc_time.tm_hour, 2), ":", itoa_padded(utc_time.tm_min, 2), ":", itoa_padded(utc_time.tm_sec, 2));
  print(&STDOUT_IO, ".", milliseconds, "\n");

  print(&STDOUT_IO, "Process ID: ", process_id, "\n");
  print(&STDOUT_IO, "Worker ID:  ", worker_id, "\n");
  print(&STDOUT_IO, "Increment:  ", increment, _endl);
  exit(0);
}
