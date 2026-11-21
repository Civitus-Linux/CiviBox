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
    xwrite("    list [all]	   	List wireless devices");
    xwrite("    block [ID|TYPE|ALL]     Disable a wireless device");
    xwrite("    unblock [ID|TYPE|ALL]   Enable a wireless device");
    xwrite("    event              	Monitor device state changes");
}

int check_type(int id, char *type, size_t size)
{
    char path[PATH_MAX];

    snprintf(path, sizeof(path), "%s/rfkill%d/type",
             RFKILL_PATH, id);

    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        perror("rfkill");
        return 1;
    }

    if (fgets(type, size, file) == NULL)
    {
        fclose(file);
        return 1;
    }

    type[strcspn(type, "\n")] = '\0';

    fclose(file);
    return 0;
}

int change_state(int id, int state)
{
    char path[PATH_MAX];

    snprintf(path, sizeof(path), "%s/rfkill%d/soft",
             RFKILL_PATH, id);

    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        perror("rfkill");
        return 1;
    }

    fprintf(file, "%d", state);

    fclose(file);
    return 0;
}

int rfkill_change(char *device, int state)
{
    DIR *dir = opendir(RFKILL_PATH);
    if (dir == NULL)
    {
        perror("rfkill");
        return 1;
    }

    struct dirent *entry;
    int result = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        int id;

        if (sscanf(entry->d_name, "rfkill%d", &id) != 1)
            continue;
	char *end;
long requested_id = strtol(device, &end, 10);

	if (*device != '\0' && *end == '\0') {
    	     if (id == requested_id && change_state(id, state) != 0) result = 1;
    	     continue;
	}

        if (strcmp(device, "all") == 0)
        {
            if (change_state(id, state) != 0)
                result = 1;
        }
        else
        {
            char type[32];

            if (check_type(id, type, sizeof(type)) != 0)
                continue;

            if (strcmp(device, type) == 0)
            {
                if (change_state(id, state) != 0)
                    result = 1;
            }
        }
    }

    closedir(dir);
    return result;
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
    char *block = getArg(argc, argv, "block", 1);
    char *unblock = getArg(argc, argv, "unblock", 1);


    if (block != NULL) return rfkill_change(block, 1);
    if (unblock != NULL) return rfkill_change(unblock, 0);
    if (list != NULL) return rfkill_list();

    usage();
    return 1;
}
