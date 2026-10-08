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

char *getArg(int argc, char **argv, const char *arg, int value)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], arg) != 0)
            continue;

        if (value && i + 1 < argc)
            return argv[i + 1];

        return (char *)1;
    }

    return NULL;
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

void xwritecolu(const char *str, int width)
{
    int len = strlen(str);

    fwrite(str, 1, len, stdout);

    for (int i = len; i < width; i++)
        fputc(' ', stdout);
}

void xwrite_raw(const char *str, int width)
{
    int len = strlen(str);

    write(STDOUT_FILENO, str, len);

    for (int i = len; i < width; i++)
        write(STDOUT_FILENO, " ", 1);
}

/* XWrite no break */
int xwritenb(const char *format, ...)
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

    return write(STDOUT_FILENO, buffer, len);
}

int xputchar(char c)
{
    return write(STDOUT_FILENO, &c, 1);
}