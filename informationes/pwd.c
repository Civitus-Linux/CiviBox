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

#include "../include/common.h"
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>

static void usage(void)
{
    xwrite("pwd [OPTION]\n");
    xwrite("-L Use the logical path");
    xwrite("-P Use the physical path");
    xwrite("-h Show this");
};

int pwd_cmd(int argc, char **argv)
{
    char path[PATH_MAX];

    char *P = getArg(argc, argv, "-P", 0);
    char *L = getArg(argc, argv, "-L", 0);
    char *h = getArg(argc, argv, "-h", 0);

    if (h)
    {
        usage();
        return 0;
    }

    if (P)
    {
        if (getcwd(path, sizeof(path)) == NULL)
            return 1;

        xwrite("%s", path);
        return 0;
    }

    if (L || argc == 1)
    {
        char *pwd = getenv("PWD");

        if (pwd == NULL)
            return 1;

        xwrite("%s", pwd);
        return 0;
    }

    return 1;
}