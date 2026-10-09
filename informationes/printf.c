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
#include <stdlib.h>
#include <string.h>

#define PRINTF_BUFFER 4096

struct printf_value
{
    char buffer[PRINTF_BUFFER];
};

static struct printf_value convertTo(const char *text, const char *format)
{
    struct printf_value value;

    value.buffer[0] = '\0';

    if (!strcmp(format, "%d") ||
        !strcmp(format, "%i"))
    {
        int n = (int)strtol(text, NULL, 10);

        snprintf(value.buffer,
                 sizeof(value.buffer),
                 format,
                 n);
    }
    else if (!strcmp(format, "%u"))
    {
        unsigned int n =
            (unsigned int)strtoul(text, NULL, 10);

        snprintf(value.buffer,
                 sizeof(value.buffer),
                 format,
                 n);
    }
    else if (!strcmp(format, "%o"))
    {
        unsigned int n =
            (unsigned int)strtoul(text, NULL, 8);

        snprintf(value.buffer,
                 sizeof(value.buffer),
                 format,
                 n);
    }
    else if (!strcmp(format, "%x") ||
             !strcmp(format, "%X"))
    {
        unsigned int n =
            (unsigned int)strtoul(text, NULL, 16);

        snprintf(value.buffer,
                 sizeof(value.buffer),
                 format,
                 n);
    }
    else if (!strcmp(format, "%f") ||
             !strcmp(format, "%e") ||
             !strcmp(format, "%E") ||
             !strcmp(format, "%g") ||
             !strcmp(format, "%G"))
    {
        double n = strtod(text, NULL);

        snprintf(value.buffer,
                 sizeof(value.buffer),
                 format,
                 n);
    }
    else if (!strcmp(format, "%c"))
    {
        snprintf(value.buffer,
                 sizeof(value.buffer),
                 format,
                 text[0]);
    }
    else if (!strcmp(format, "%s"))
    {
        snprintf(value.buffer,
                 sizeof(value.buffer),
                 "%s",
                 text);
    }
    else
    {
        snprintf(value.buffer,
                 sizeof(value.buffer),
                 "%s",
                 text);
    }

    return value;
}

int printf_cmd(int argc, char **argv)
{
    if (argc < 2)
        return 0;

    const char *format = argv[1];
    int arg = 2;

    for (int i = 0; format[i] != '\0'; i++)
    {
        if (format[i] != '%')
        {
            if (format[i] == '\\' && format[i + 1] != '\0')
            {
                i++;

                switch (format[i])
                {
                case 'n':
                    xputchar('\n');
                    break;

                case 't':
                    xputchar('\t');
                    break;

                case 'r':
                    xputchar('\r');
                    break;

                case 'b':
                    xputchar('\b');
                    break;

                case 'a':
                    xputchar('\a');
                    break;

                case 'f':
                    xputchar('\f');
                    break;

                case 'v':
                    xputchar('\v');
                    break;

                case '\\':
                    xputchar('\\');
                    break;

                default:
                    xputchar('\\');
                    xputchar(format[i]);
                    break;
                }

                continue;
            }

            xputchar(format[i]);
            continue;
        }

        i++;

        if (format[i] == '%')
        {
            xputchar('%');
            continue;
        }

        char spec[3];

        spec[0] = '%';
        spec[1] = format[i];
        spec[2] = '\0';

        if (arg >= argc)
            continue;

        struct printf_value value =
            convertTo(argv[arg++], spec);

        xwritenb("%s", value.buffer);
    }

    return 0;
}