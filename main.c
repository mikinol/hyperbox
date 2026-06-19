#include "lib.h"

#include "./commands/cat.h"
#include "./commands/cp.h"
#include "./commands/discord_snowflake_parse.h"
#include "./commands/echo.h"
#include "./commands/mkdir.h"
#include "./commands/password_gen.h"
#include "./commands/tee.h"
#include "./commands/terminal_gameoflife.h"
#include "./commands/wc.h"

[[noreturn]] int main(int argc, char **argv) {
  if (unlikely(argc < 1))
    goto incorrect;

  const char *cmd = get_basename(argv[0]);
  unsigned long len = strlen(cmd);

  typedef void (*worker_fn)(int, char **);
  worker_fn func = NULL;

  if (len < 8) {
    uint64_t cmd_v = 0;
    memcpy(&cmd_v, cmd, len);

    if (cmd_v == STR_TO_UINT64("cat")) {
      func = do_cat;
    } else if (cmd_v == STR_TO_UINT64("mkdir")) {
      func = do_mkdir;
    } else if (cmd_v == STR_TO_UINT64("echo")) {
      func = do_echo;
    } else if (cmd_v == STR_TO_UINT64("wc")) {
      func = do_wc;
    } else if (cmd_v == STR_TO_UINT64("tee")) {
      func = do_tee;
    } else if (cmd_v == STR_TO_UINT64("cp")) {
      func = do_cp;
    }
  } else {
    if (strcmp(cmd, "password_gen") == 0) {
      func = do_password_gen;
    } else if (strcmp(cmd, "discord_snowflake_parse") == 0) {
      func = do_discord_snowflake_parse;
    } else if (strcmp(cmd, "terminal_gameoflife") == 0) {
      func = do_terminal_gameoflife;
    }
  }

  if (likely(func))
    func(argc, argv);
incorrect:
  WRITE_LITERAL(STDERR_FILENO, "Incorrect cmd");
  exit(1);
}
