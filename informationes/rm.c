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
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int r, f;

static void usage(void)
{
    xwrite("mv [OPTION] SOURCE... DIRECTORY\n");
    xwrite(" -f ignore files missing and don't ask");
    xwrite(" -i ask withouth deleting");
    xwrite(" -r[-R] remove directory recursively");
};

static int ask(const char *file)
{
    char answer[8];

    xwrite("rm: delete %s? (y/n) [n] ", file);

    if (fgets(answer, sizeof(answer), stdin) == NULL)
        return 0;

    return answer[0] == 'y' || answer[0] == 'Y';
}

static int rm(const char *file)
{
    struct stat st;

    if (stat(file, &st) < 0) {
        if (f && errno == ENOENT)
            return 0;

        perror("rm");
        return 1;
    }

    if (S_ISDIR(st.st_mode)) {
        if (!r) {
            xwrite("rm: %s: is a directory", file);
            return 1;
        }

        if (rmdir(file) < 0) {
            perror("rm");
            return 1;
        }
    } else {
        if (unlink(file) < 0) {
            perror("rm");
            return 1;
        }
    }

    return 0;
}

int rm_cmd(int argc, char **argv)
{
    if (argc < 2)
    {
        usage();
        return 0;
    }

    const char *file = NULL;

    int i = getArg(argc, argv, "-i");
    f = getArg(argc, argv, "-f");
    r = getArg(argc, argv, "-r") || getArg(argc, argv, "-R");

    for (int j = 1; j < argc; j++)
    {
        if (argv[j][0] == '-')
            continue;

        if (file == NULL)
            file = argv[j];
    }

    if (file == NULL)
    {
        usage();
        return 1;
    }

    if (i && !f && access(file, F_OK) == 0)
    {
        if (!ask(file))
            return 0;
    };

    return rm(file);
}