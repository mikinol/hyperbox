#include "lib.h"

#include "./commands/cat.h"
#include "./commands/cp.h"
#include "./commands/discord_snowflake_parse.h"
#include "./commands/echo.h"
#include "./commands/mkdir.h"
#include "./commands/password_gen.h"
#include "./commands/wc.h"

int main(int argc, char **argv) {
  if (argc < 1)
    exit(1);

  const char *cmd = get_basename(argv[0]);
  if (strcmp(cmd, "cat") == 0) {
    do_cat(argc, argv);
  } else if (strcmp(cmd, "mkdir") == 0) {
    do_mkdir(argc, argv);
  } else if (strcmp(cmd, "echo") == 0) {
    do_echo(argc, argv);
  } else if (strcmp(cmd, "wc") == 0) {
    do_wc(argc, argv);
  } else if (strcmp(cmd, "cp") == 0) {
    do_cp(argc, argv);
  } else if (strcmp(cmd, "password_gen") == 0) {
    do_password_gen(argc, argv);
  } else if (strcmp(cmd, "discord_snowflake_parse") == 0) {
    do_discord_snowflake_parse(argc, argv);
  }

  WRITE_LITERAL(STDERR_FILENO, "Incorrect cmd");
  exit(1);
}
