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

#include "common.h"
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RFKILL_PATH "/sys/class/rfkill"

static void usage(void)
{
    xwrite("Usage: rfkill [OPTION] [ID]\n");
    xwrite("    list [all]       List wireless devices");
    xwrite("    block ID         Disable a wireless device");
    xwrite("    unblock ID       Enable a wireless device");
    xwrite("    block all        Disable all wireless devices");
    xwrite("    unblock all      Enable all wireless devices");
    xwrite("    event            Monitor device state changes");
}

int read(const char *path, char *buf, size_t size)
{
    FILE *file = fopen(path, "r");
    
    if (file == NULL) return 1;

    if (fgets(buf, size, file) == NULL)
    {
        fclose(file);
        return 1;
    }

    buf[strcspn(buf, "\n")] = '\0';

    fclose(file);
    return 0;
}

int rfkill_list()
{
    DIR *dir = opendir(RFKILL_PATH);
    if (dir == NULL)
    {
        perror("rfkill: ");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
	if (entry->d_name[0] == '.') continue;
        char buf[128];
        char path[PATH_MAX];

        snprintf(path, sizeof(path), "%s/%s/index", RFKILL_PATH, entry->d_name);
        if (read(path, buf, sizeof(buf)) == 0) printf("%s: ", buf);
        
        snprintf(path, sizeof(path), "%s/%s/name", RFKILL_PATH, entry->d_name);
	if (read(path, buf, sizeof(buf)) == 0) printf("%s: ", buf);

        snprintf(path, sizeof(path), "%s/%s/type", RFKILL_PATH, entry->d_name);
        if (read(path, buf, sizeof(buf)) == 0) printf("%s\n", buf);

        snprintf(path, sizeof(path), "%s/%s/soft", RFKILL_PATH, entry->d_name);
        if (read(path, buf, sizeof(buf)) == 0) printf("	Soft blocked: %s\n", atoi(buf) == 1 ? "yes" : "no");

        snprintf(path, sizeof(path), "%s/%s/hard", RFKILL_PATH, entry->d_name);
        if (read(path, buf, sizeof(buf)) == 0) printf("	Hard blocked: %s\n", atoi(buf) == 1 ? "yes": "no");
    }

    closedir(dir);
    return 0;
}

int rfkill_cmd(int argc, char **argv)
{
    if (argc < 2)
    {
        usage();
        return 1;
    }

    char *list = getArg(argc, argv, "list", 0); 

    if (list != 0) return rfkill_list();

    usage();
    return 1;
}
