#include "nolibc/nolibc.h"

#ifndef _HYPERBOX_LIB_H
#define _HYPERBOX_LIB_H

// Buffers sizes
#define CAT_BUFFER_SIZE 16384
#define CAT_BUFFER_KEEP 512
#define COPY_BUFFER_SIZE 8192
#define WC_BUFFER_SIZE 8192
#define STDERR_WRITEBUFFER_SIZE 4096

// Print
static char STDERR_WRITEBUFFER[STDERR_WRITEBUFFER_SIZE];
static unsigned long STDERR_WRITEBUFFER_POS;

static inline void errprint_array(const char *arr, unsigned long length) {
  while (STDERR_WRITEBUFFER_POS + STDERR_WRITEBUFFER_POS > STDERR_WRITEBUFFER_SIZE) {
    unsigned long tocopy = STDERR_WRITEBUFFER_SIZE - STDERR_WRITEBUFFER_POS;
    memcpy(STDERR_WRITEBUFFER, arr, tocopy);
    write(STDERR_FILENO, STDERR_WRITEBUFFER, STDERR_WRITEBUFFER_SIZE);
    STDERR_WRITEBUFFER_POS = 0;
  }

  memcpy(STDERR_WRITEBUFFER + STDERR_WRITEBUFFER_POS, arr, length);
  STDERR_WRITEBUFFER_POS += length;
}
static inline void errprint_string(const char *str) { errprint_array(str, strlen(str)); }
static inline void errprint_long(long num) { errprint_string(itoa(num)); }
static inline void errprint_flush() {
  write(STDERR_FILENO, STDERR_WRITEBUFFER, STDERR_WRITEBUFFER_POS);
  STDERR_WRITEBUFFER_POS = 0;
}

#define errprint_literal(str_literal) errprint_array("" str_literal, sizeof(str_literal) - 1)

// Utils
#define WRITE_LITERAL(fd, str_literal) write(fd, "" str_literal, sizeof(str_literal) - 1)

static inline const char *get_basename(const char *path) {
  const char *base = path;
  while (*path) {
    if (*path == '/')
      base = path + 1;
    path++;
  }
  return base;
}

#endif
