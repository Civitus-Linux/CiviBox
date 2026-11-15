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
