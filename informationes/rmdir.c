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
#include <errno.h>
#include <stdio.h>
#include <string.h>

static void usage(void)
{
    xwrite("rmdir [OPTION] DIRECTORY");
    xwrite(" -p       remove parent directories as needed");
}

static int rmdirr(const char *folder, char *p)
{
    char path[PATH_MAX];
    char *slash;

    if (!p)
    {
        if (rmdir(folder) < 0)
        {
            perror("rmdir");
            return 1;
        }

        return 0;
    }

    if (strlen(folder) >= sizeof(path))
    {
        xwrite("rmdir: path too long");
        return 1;
    }

    strcpy(path, folder);

    if (rmdir(path) < 0)
    {
        perror("rmdir");
        return 1;
    }

    for (slash = strrchr(path, '/'); slash; slash = strrchr(path, '/'))
    {
        *slash = '\0';

        if (!*path)
            break;

        if (rmdir(path) < 0)
            break;
    }

    return 0;
}

int rmdir_cmd(int argc, char **argv)
{
    if (argc == 1)
    {
        usage();
        return 1;
    }

    const char *folder = NULL;
    char *p = getArg(argc, argv, "-p", 0);

    for (int j = 1; j < argc; j++)
    {
        if (argv[j][0] == '-')
            continue;

        folder = argv[j];
        break;
    }

    if (folder == NULL)
    {
        usage();
        return 1;
    }

    return rmdirr(folder, p);
}