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

static void usage(void)
{
    xwrite("base command blabla\n");
    xwrite("-n say n");
    xwrite("-s msg   say <msg> message");
};

int base_cmd(int argc, char **argv)
{
    if (argc == 1)
    {
        usage();
        return 0;
    }

    char *n = getArg(argc, argv, "-n", 0); 
    /* 
    if the last argument is 1, will return the value
    Example: -n test
    getArg(argc, argv, "-n", 0) -> test
    getArg(argc, argv, "-n", 1) -> exist or not
    */

    return 0;
}