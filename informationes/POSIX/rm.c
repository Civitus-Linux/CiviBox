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
#include <string.h>
#include "usage.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <stdlib.h>

static void usage(void)
{
    xwrite("mv [OPTION] SOURCE... DIRECTORY\n");
    xwrite(" -f ignore files missing and don't ask");
    xwrite(" -i ask withouth deleting");
    xwrite(" -r[-R] remove directory recursively");
};

static int ask(const char *file)
{
    char answer[8];

    xwrite("rm: delete %s? (y/n) [n] ", file);

    if (fgets(answer, sizeof(answer), stdin) == NULL)
        return 0;

    return answer[0] == 'y' || answer[0] == 'Y';
}


static int rm(const char *path, char *f, int r)
{
    struct stat st;

    if (lstat(path, &st) < 0)
    {
        if (f && errno == ENOENT)
            return 0;

        perror(path);
        return 1;
    }

    if (!S_ISDIR(st.st_mode))
    {
        if (unlink(path) < 0)
        {
            perror(path);
            return 1;
        }

        return 0;
    }

    if (!r)
    {
        fprintf(stderr, "rm: %s: is a directory\n", path);
        return 1;
    }

    DIR *dir = opendir(path);

    if (!dir)
    {
        perror(path);
        return 1;
    }

    struct dirent *entry;
    int status = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (!strcmp(entry->d_name, ".") ||
            !strcmp(entry->d_name, ".."))
            continue;

        char child[PATH_MAX];

        int n = snprintf(child, sizeof(child),
                         "%s/%s", path, entry->d_name);

        if (n < 0 || (size_t)n >= sizeof(child))
        {
            fprintf(stderr, "rm: path too long\n");
            status = 1;
            continue;
        }

        if (rm(child, f, 1) != 0)
            status = 1;
    }

    closedir(dir);

    if (status)
        return 1;

    if (rmdir(path) < 0)
    {
        perror(path);
        return 1;
    }

    return 0;
}

int rm_cmd(int argc, char **argv)
{
    if (argc < 2)
    {
        usage();
        return 0;
    }

    const char *file = NULL;

    char *i = getArg(argc, argv, "-i", 0);
    char *f = getArg(argc, argv, "-f", 0);
    int r = getArg(argc, argv, "-r", 0) || getArg(argc, argv, "-R", 0);

    for (int j = 1; j < argc; j++)
    {
        if (argv[j][0] == '-')
            continue;

        if (file == NULL)
            file = argv[j];
    }

    if (file == NULL)
    {
        usage();
        return 1;
    }

    if (i && !f && access(file, F_OK) == 0)
    {
        if (!ask(file))
            return 0;
    };

    return rm(file, f, r);
}
