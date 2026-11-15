#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include "../include/usage.h"

void usage()
{
    puts("Usage: basename STRING [SUFFIX]\n");
    puts("	-h	Show this help message");
}

void basenamed(char *name, char *suffix)
{
    size_t name_len, suffix_len;
    name_len = strlen(name);
    suffix_len = strlen(suffix);

    if (suffix_len > 0 && suffix_len <= name_len && strcmp(name + name_len - suffix_len, suffix) == 0) name[name_len-suffix_len] = '\0';
}

int basename_cmd(int argc, char **argv) {
    if (argc < 2 || argc > 3)
    {
        usage();
        return 1;
    }
    char *name = basename(argv[1]);

    if (argc == 3) basenamed(name, argv[2]);

    puts(name);

    return 0;
}
