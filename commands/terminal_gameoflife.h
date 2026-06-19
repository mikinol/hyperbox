#include "../lib.h"
struct winsize w;

volatile int stop = 0;
volatile int resized = 0;
volatile int new_cols = 0;
volatile int new_rows = 0;

static void handle_signal(int signum) {
  if (signum == SIGWINCH) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
      new_cols = w.ws_col;
      new_rows = w.ws_row;
      resized = 1;
    }
  } else {
    stop = 1;
  }
}

#define CELL(grid, i, j, cols) (grid)[(i) * (cols) + (j)]

static inline void fill_random(char *grid, const long size) {
  uint64_t rand_state = time(NULL);

  for (long i = 0; i < size; i++) {
    rand_state ^= rand_state << 13;
    rand_state ^= rand_state >> 7;
    rand_state ^= rand_state << 17;

    grid[i] = rand_state % 2 == 0;
  }
}

static inline int max(int arg1, int arg2) { return arg1 > arg2 ? arg1 : arg2; }

void print_grid(char *grid, unsigned short cols, unsigned short rows) {
  print(&STDOUT_IO, "\033[H\033[J");
  for (unsigned short i = 0; i < rows; i++) {
    for (unsigned short j = 0; j < cols; j++) {
      if (CELL(grid, i, j, cols)) {
        print(&STDOUT_IO, "#");
      } else {
        print(&STDOUT_IO, " ");
      }
    }

    if (i + 1 < rows)
      print(&STDOUT_IO, "\n");
  }
  print(&STDOUT_IO, _flush);
}

static inline void resize_grid(char *oldgrid, char *grid, const unsigned short cols, const unsigned short rows,
                               const unsigned short new_cols, const unsigned short new_rows) {
  int max_i = (rows < new_rows) ? rows : new_rows;

  if (oldgrid == grid) {
    if (cols < new_cols) {
      for (int i = max_i - 1; i > 0; i--)
        memmove(&CELL(grid, i, 0, new_cols), &CELL(grid, i, 0, cols), cols * sizeof(char));

      for (int i = 0; i < max_i; i++)
        memset(&CELL(grid, i, cols, new_cols), 0, (new_cols - cols) * sizeof(char));
    } else if (cols > new_cols) {
      for (int i = 1; i < max_i; i++)
        memmove(&CELL(grid, i, 0, new_cols), &CELL(grid, i, 0, cols), new_cols);
    }

    if (new_rows > rows)
      memset(&CELL(grid, rows, 0, new_cols), 0, (new_rows - rows) * new_cols * sizeof(char));

  } else {
    int max_cols = (cols < new_cols) ? cols : new_cols;
    for (int i = 0; i < rows; i++)
      memmove(&CELL(grid, i, 0, new_cols), &CELL(oldgrid, i, 0, cols), max_cols);

    if (cols < new_cols)
      for (int i = 0; i < rows; i++)
        memset(&CELL(grid, i, cols, new_cols), 0, (new_cols - cols) * sizeof(char));

    if (new_rows > rows)
      memset(&CELL(grid, rows, 0, new_cols), 0, (new_rows - rows) * new_cols * sizeof(char));
  }
}

void run_tick(const char *grid, char *temp_grid, const unsigned short cols, const unsigned short rows) {
  for (unsigned short i = 0; i < rows; i++) {
    for (unsigned short j = 0; j < cols; j++) {
      unsigned char val = 0;
      if (i != 0)
        val += CELL(grid, i - 1, j, cols);
      if (j != 0)
        val += CELL(grid, i, j - 1, cols);
      if (i != rows - 1)
        val += CELL(grid, i + 1, j, cols);
      if (j != cols - 1)
        val += CELL(grid, i, j + 1, cols);
      if (i != 0 && j != 0)
        val += CELL(grid, i - 1, j - 1, cols);
      if (i != rows - 1 && j != 0)
        val += CELL(grid, i + 1, j - 1, cols);
      if (i != 0 && j != cols - 1)
        val += CELL(grid, i - 1, j + 1, cols);
      if (i != rows - 1 && j != cols - 1)
        val += CELL(grid, i + 1, j + 1, cols);

      if (CELL(grid, i, j, cols))
        CELL(temp_grid, i, j, cols) = val == 2 || val == 3;
      else
        CELL(temp_grid, i, j, cols) = val == 3;
    }
  }
}

static void my_sigrestorer(void) { syscall(SYS_rt_sigreturn); }

uintptr_t page_align_up(uintptr_t addr) { return (addr + 4095) & ~4095; }

[[noreturn]] static inline void do_terminal_gameoflife(int argc, char **argv) {
  if (unlikely(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)) {
    print(&STDERR_IO, "Cannot get terminal size: ", _errno, _endl);
    exit(1);
  }

  struct sigaction act;
  act.sa_handler = handle_signal;
  act.sa_flags = SA_RESTART | SA_RESTORER;
  act.sa_mask = 0;
  act.sa_restorer = my_sigrestorer;

  syscall(SYS_rt_sigaction, SIGINT, &act, NULL, 8);
  syscall(SYS_rt_sigaction, SIGTERM, &act, NULL, 8);
  syscall(SYS_rt_sigaction, SIGWINCH, &act, NULL, 8);

  unsigned short int cols = w.ws_col;
  unsigned short int rows = w.ws_row;

  int size = cols * rows;
  void *initial_brk = _sys_brk(NULL);
  char *grid = initial_brk;
  char *temp_grid = grid + size;
  void *brk = _sys_brk(temp_grid + max(size, 8 * 1024));
  if (brk < 0 || initial_brk < 0) {
    print(&STDERR_IO, "Cannot allocate memory: ", _errno, _endl);
    exit(1);
  }

  fill_random(grid, size);

  static const int64_t default_sleep = 100 * 1000 * 1000;
  struct timespec req = {.tv_sec = 0, .tv_nsec = default_sleep};
  while (!stop) {
    if (unlikely(resized)) {
      resized = 0;
      if (cols == new_cols && rows == new_rows)
        continue;

      if (grid == initial_brk) {
        char *temp = grid;
        grid = temp_grid;
        temp_grid = temp;
        memcpy(grid, initial_brk, size);
      }

      size = new_cols * new_rows;
      if (unlikely((long)brk < (long)initial_brk + (size * 2))) {
        brk = _sys_brk((void *)page_align_up((unsigned long)initial_brk + (size * 2)));
        if (brk < 0) {
          print(&STDERR_IO, "Cannot allocate memory: ", _errno, _endl);
          exit(1);
        }
      }
      resize_grid(grid, initial_brk, cols, rows, new_cols, new_rows);

      grid = initial_brk;
      temp_grid = grid + size;
      cols = new_cols;
      rows = new_rows;
    }

    if (req.tv_nsec == default_sleep) {
      run_tick(grid, temp_grid, cols, rows);
      char *temp = grid;
      grid = temp_grid;
      temp_grid = temp;
    }

    print_grid(grid, cols, rows);

    if (likely(_syscall(SYS_nanosleep, &req, &req) == 0)) {
      req.tv_nsec = default_sleep;
    }
  }

  exit(0);
}
