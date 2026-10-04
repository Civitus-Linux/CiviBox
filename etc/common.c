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

#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>

int getArg(int argc, char **argv, const char *arg)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], arg) == 0)
            return 1;
    }

    return 0;
}

int xwrite(const char *format, ...)
{
    char buffer[4096];

    va_list args;
    va_start(args, format);

    int len = vsnprintf(buffer,
                        sizeof(buffer) - 1,
                        format,
                        args);

    va_end(args);

    if (len < 0)
        return -1;

    buffer[len++] = '\n';

    return write(STDOUT_FILENO, buffer, len);
}

int xputchar(char c)
{
    return write(STDOUT_FILENO, &c, 1);
}