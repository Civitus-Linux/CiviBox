#include <stdio.h>
#include <libgen.h>
#include "../include/usage.h"

static void usage()
{
    puts("Usage: dirname [STRING]\n");
    puts("      -h      Show this help message.");
}

int dirname_cmd(int argc, char **argv)
{
    if (argc != 2)
    {
        usage();
        return 1;
    }

    puts(dirname(argv[1]));

    return 0;
}
