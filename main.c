/*
 * CiviBox
 *
 * Copyright (C) 2026
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#include "include/config.h"
#include "include/common.h"
#include <string.h>
char *version = "0.0.1";

void usage() {
    xwrite("CiviBox - v%s", version);
    xwrite("Licensed under GPL-2.0-or-later, this software is open sourced");
    xwrite("Usage: civibox [command] [args]");
    xwrite("[");
    for (int i = 0; commands[i].name != NULL; i++)
        xwrite("  %s,", commands[i].name);
    xwrite("]");
};

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        return 0;
    }

    for (int i = 0; commands[i].name != NULL; i++)
    {
        if (strcmp(argv[1], commands[i].name) == 0)
        {
            return commands[i].func(argc - 1, argv + 1);
        }
    }

    return 1;
}