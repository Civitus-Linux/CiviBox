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
#include <unistd.h>
#include <stdio.h>

static void usage(void)
{
    xwrite("mv [OPTION] SOURCE... DIRECTORY\n");
    xwrite(" -f overwrite without asking");
    xwrite(" -i ask before overwriting");
};

static int ask(const char *dest, const char *src)
{
    char answer[8];

    xwrite("mv: overwrite %s with %s? (y/n) [n] ", dest, src);

    if (fgets(answer, sizeof(answer), stdin) == NULL)
        return 0;

    return answer[0] == 'y' || answer[0] == 'Y';
}

static int mv(const char *dest, const char *src) {
    if (rename(src, dest) != 0) {
        xwrite("mv: rename failed");
        return 1;
    }

    return 0;
}


int mv_cmd(int argc, char **argv)
{
    if (argc < 3)
    {
        usage();
        return 0;
    }

    const char *src = NULL;
    const char *dest = NULL;

    int i = getArg(argc, argv, "-i");
    int f = getArg(argc, argv, "-f");

    for (int j = 1; j < argc; j++)
    {
        if (argv[j][0] == '-')
            continue;

        if (src == NULL)
            src = argv[j];
        else
        {
            dest = argv[j];
            break;
        }
    }

    if (src == NULL || dest == NULL) {
        usage();
        return 1;
    }

    if (i && !f && access(dest, F_OK) == 0) {
        if (!ask(dest, src)) return 0;
    };

    return mv(dest, src);
}