#include <stdlib.h>

int exit_cmd(int argc, char *argv[]) {
    if (argc < 2) return 1;
    return atoi(argv[1]);
}
