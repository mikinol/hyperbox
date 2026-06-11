#include "nolibc/nolibc.h"

int main(int argc, char **argv) { write(STDOUT_FILENO, "Hello World!\n", 13); }
