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
#include <stddef.h>
#include <string.h>

static void usage(void)
{
    xwrite("echo [OPTION]... [STRING]...\n");
    xwrite(" -n Don't add a new line");
    xwrite(" -e Interpret escape sequences");
    xwrite(" -E Don't interpret escape sequences");
    xwrite(" -h Show this");
}

static void pescape(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] != '\\')
        {
            xputchar(str[i]);
            continue;
        }

        i++;

        switch (str[i])
        {
        case 'n':
            xputchar('\n');
            break;

        case 't':
            xputchar('\t');
            break;

        case '\\':
            xputchar('\\');
            break;

        case 'a':
            xputchar('\a');
            break;

        case 'b':
            xputchar('\b');
            break;

        default:
            xputchar('\\');
            xputchar(str[i]);
            break;
        }
    }
}

int echo_cmd(int argc, char **argv)
{
    char *n = getArg(argc, argv, "-n", 0);
    char *e = getArg(argc, argv, "-e", 0);
    char *E = getArg(argc, argv, "-E", 0);
    char *h = getArg(argc, argv, "-h", 0);

    if (h)
    {
        usage();
        return 0;
    }

    int first = 1;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-n") ||
            !strcmp(argv[i], "-e") ||
            !strcmp(argv[i], "-E") ||
            !strcmp(argv[i], "-h"))
            continue;

        if (!first)
            xwritenb(" ");

        if (e && !E)
            pescape(argv[i]);
        else
            xwritenb("%s", argv[i]);

        first = 0;
    }

    if (!n)
        xwrite("");

    return 0;
}