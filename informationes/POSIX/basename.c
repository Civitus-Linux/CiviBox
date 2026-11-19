/*
 * CiviBox - Minimal userspace toolkit for Linux
 *
 * Copyright (C) 2026 Pietro Carlini
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include "usage.h"

static void usage()
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
