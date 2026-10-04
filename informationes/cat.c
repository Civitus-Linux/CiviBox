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
#include <stdio.h>

static void usage(void)
{
    xwrite("cat [OPTION] [FILE]\n");
    xwrite(" -n Numbers all lines");
    xwrite(" -u Does not use buffer at output");
};

int cat_cmd(int argc, char **argv)
{
    if (argc == 1)
    {
        usage();
        return 0;
    }

    int n = getArg(argc, argv, "-n");
    int u = getArg(argc, argv, "-u");

    if (u)
        setbuf(stdout, NULL);

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
            continue;

        FILE *file = fopen(argv[i], "r");

        if (!file)
            return 1;

        int c;
        int line = 1;
        int start = 1;

        while ((c = fgetc(file)) != EOF)
        {
            if (start && n)
            {
                xwrite("%d ", line++);
                start = 0;
            }

            if (u)
                xputchar(c);
            else
                putchar(c);

            if (c == '\n')
                start = 1;
        }

        fclose(file);
    }

    return 0;
}