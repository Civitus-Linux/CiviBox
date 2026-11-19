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
#include <unistd.h>
#include <stdio.h>

char *version = "0.3.3";

void usage() {
    xwrite("CiviBox - v%s", version);
    xwrite("Licensed under GPL-2.0-or-later, this software is open sourced");
    xwrite("Usage: civibox [command] [args]");
    xwritenb("[");
    for (int i = 0; commands[i].name != NULL; i++) {
        xwritenb("  %s", commands[i].name);
        if (commands[i+1].name != NULL) xwritenb(",");
    }
    xwrite(" ]");
};

int main(int argc, char **argv) {
    char *name = strrchr(argv[0], '/');
    name = name ? name + 1 : argv[0];

    if (!strcmp(name, "civibox")) {
        if (argc < 2)
        {
            usage();
            return 1;
        }

        name = argv[1];
        argc--;
        argv++;
    }

    for (int i = 0; commands[i].name; i++) {
        if (!strcmp(name, commands[i].name))
            return commands[i].func(argc, argv);
    }

    usage();
    return 1;
}
