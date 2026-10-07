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
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

char *m, *p;

static void usage(void)
{
    xwrite("mkdir [OPTION] DIRECTORY");
    xwrite(" -m mode  set folder mode bits");
    xwrite(" -p       create parent directories as needed");
}

static int mkkdir(const char *folder, mode_t mode)
{
    char path[PATH_MAX];
    char *slash;

    if (!p)
    {
        if (mkdir(folder, mode) < 0)
        {
            perror("mkdir");
            return 1;
        }

        return 0;
    }

    if (strlen(folder) >= sizeof(path))
    {
        xwrite("mkdir: path too long");
        return 1;
    }

    strcpy(path, folder);

    for (slash = path + 1; *slash; slash++)
    {
        if (*slash != '/')
            continue;

        *slash = '\0';

        if (*path && mkdir(path, mode) < 0 && errno != EEXIST)
        {
            perror("mkdir");
            return 1;
        }

        *slash = '/';
    }

    if (mkdir(path, mode) < 0 && errno != EEXIST)
    {
        perror("mkdir");
        return 1;
    }

    return 0;
}

int mkdir_cmd(int argc, char **argv)
{
    if (argc == 1)
    {
        usage();
        return 0;
    }

    const char *folder = NULL;
    mode_t mode = 0777;
    p = getArg(argc, argv, "-p", 1);

    for (int j = 1; j < argc; j++)
    {
        if (strcmp(argv[j], "-m") == 0)
        {
            if (j + 1 >= argc)
            {
                xwrite("mkdir: option requires an argument -- m");
                return 1;
            }

            mode = strtol(argv[++j], NULL, 8);
            continue;
        }

        if (argv[j][0] == '-')
            continue;

        if (folder == NULL)
            folder = argv[j];
    }

    if (folder == NULL)
    {
        usage();
        return 1;
    }

    return mkkdir(folder, mode);
}