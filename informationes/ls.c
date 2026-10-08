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

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

struct ls_entry
{
    char name[NAME_MAX];
    struct stat st;
};

struct options
{
    int a;
    int A;
    int C;
    int d;
    int F;
    int l;
    int R;
    int S;
    int t;
};

int d(const void *a, const void *b)
{
    int num_a = *(const int *)a;
    int num_b = *(const int *)b;
    return (num_b > num_a) - (num_b < num_a);
}

static void usage(void)
{
    xwrite("ls [OPTION] [FILE]");
    xwrite(" -a show hidden files");
    xwrite(" -A show hidden files except . and ..");
    xwrite(" -C list entries in columns");
    xwrite(" -d list directory itself");
    xwrite(" -F append indicators to names");
    xwrite(" -l use long listing format");
    xwrite(" -R list subdirectories recursively");
    xwrite(" -S sort by file size");
    xwrite(" -t sort by modification time");
}

static int valid_option(const char *arg)
{
    if (arg[0] != '-')
        return 1;

    if (strcmp(arg, "--") == 0)
        return 1;

    for (int i = 1; arg[i] != '\0'; i++)
    {
        switch (arg[i])
        {
        case 'a':
        case 'A':
        case 'C':
        case 'd':
        case 'F':
        case 'l':
        case 'R':
        case 'S':
        case 't':
        case '1':
            break;

        default:
            return 0;
        }
    }

    return 1;
}

static int ls(const char *folder, struct options *opt)
{
    DIR *dir;
    struct dirent *entry;

    if (opt->d)
    {
        xwrite("%s", folder);
        return 0;
    }

    dir = opendir(folder);

    if (!dir)
    {
        perror("ls");
        return 1;
    }

    int col = 0;
    int columns = 1;
    int width = 15;

    if (opt->C)
    {
        struct winsize ws;

        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 &&
            ws.ws_col > 0)
        {
            columns = ws.ws_col / width;

            if (columns < 1)
                columns = 1;
        }
    }

    struct ls_entry *entries = NULL;
    int count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (!opt->a && !opt->A && entry->d_name[0] == '.')
            continue;

        if (opt->A &&
            (strcmp(entry->d_name, ".") == 0 ||
             strcmp(entry->d_name, "..") == 0))
            continue;

        char path[PATH_MAX];
        struct stat st;

        snprintf(path, sizeof(path), "%s/%s",
                 folder, entry->d_name);

        if (lstat(path, &st) == -1)
            continue;

        if (opt->S || opt->t)
        {
            entries = realloc(entries, (count + 1) * sizeof(*entries));

            if (!entries)
            {
                closedir(dir);
                return 1;
            }

            strcpy(entries[count].name, entry->d_name);
            entries[count].st = st;
            count++;

            continue;
        }

        if (opt->l)
        {
            char mode[11];
            char date[64];

            mode[0] = S_ISDIR(st.st_mode) ? 'd' : S_ISLNK(st.st_mode) ? 'l'
                                                                      : '-';

            mode[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
            mode[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
            mode[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';

            mode[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
            mode[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
            mode[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';

            mode[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
            mode[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
            mode[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';

            mode[10] = '\0';

            ctime_r(&st.st_mtime, date);
            date[strcspn(date, "\n")] = '\0';

            xwrite("%s %lu %u %u %ld %s %s",
                   mode,
                   (unsigned long)st.st_nlink,
                   st.st_uid,
                   st.st_gid,
                   (long)st.st_size,
                   date,
                   entry->d_name);

            continue;
        }

        if (opt->C)
        {
            xwrite_raw(entry->d_name, width);

            col++;

            if (col >= columns)
            {
                xputchar('\n');
                col = 0;
            }

            continue;
        }

        if (opt->F)
        {
            if (S_ISDIR(st.st_mode))
                xwrite("%s/", entry->d_name);
            else if (S_ISLNK(st.st_mode))
                xwrite("%s@", entry->d_name);
            else if (st.st_mode & S_IXUSR)
                xwrite("%s*", entry->d_name);
            else
                xwrite("%s", entry->d_name);

            continue;
        }

        xwrite("%s", entry->d_name);
    }

    closedir(dir);

    if (opt->S)
    {
        for (int i = 0; i < count - 1; i++)
        {
            for (int j = i + 1; j < count; j++)
            {
                if (entries[j].st.st_size > entries[i].st.st_size)
                {
                    struct ls_entry tmp = entries[i];
                    entries[i] = entries[j];
                    entries[j] = tmp;
                }
            }
        }

        for (int i = 0; i < count; i++)
            xwrite("%s %ld", entries[i].name, (long)entries[i].st.st_size);

        free(entries);
    }

    if (opt->t)
    {
        for (int i = 0; i < count - 1; i++)
        {
            for (int j = i + 1; j < count; j++)
            {
                if (entries[j].st.st_mtime > entries[i].st.st_mtime)
                {
                    struct ls_entry tmp = entries[i];
                    entries[i] = entries[j];
                    entries[j] = tmp;
                }
            }
        }

        for (int i = 0; i < count; i++)
        {
            char date[64];

            ctime_r(&entries[i].st.st_mtime, date);
            date[strcspn(date, "\n")] = '\0';

            xwrite("%s %s", entries[i].name, date);
        }

        free(entries);
    }

    if (opt->C && col != 0)
        xputchar('\n');

    if (opt->R)
    {
        dir = opendir(folder);

        if (!dir)
            return 1;

        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
                continue;

            if (!opt->a && !opt->A && entry->d_name[0] == '.')
                continue;

            char path[PATH_MAX];
            struct stat st;

            snprintf(path, sizeof(path), "%s/%s",
                     folder, entry->d_name);

            if (lstat(path, &st) == -1)
                continue;

            if (S_ISDIR(st.st_mode))
            {
                xwrite("%s:", path);
                ls(path, opt);
                xputchar('\n');
            }
        }

        closedir(dir);
    }

    return 0;
}

int ls_cmd(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (!valid_option(argv[i]))
        {
            usage();
            return 1;
        }
    }

    struct options opt = {
        .a = getArg(argc, argv, "-a", 0) != NULL,
        .A = getArg(argc, argv, "-A", 0) != NULL,
        .C = getArg(argc, argv, "-C", 0) != NULL,
        .d = getArg(argc, argv, "-d", 0) != NULL,
        .F = getArg(argc, argv, "-F", 0) != NULL,
        .l = getArg(argc, argv, "-l", 0) != NULL,
        .R = getArg(argc, argv, "-R", 0) != NULL,
        .S = getArg(argc, argv, "-S", 0) != NULL,
        .t = getArg(argc, argv, "-t", 0) != NULL,
    };

    const char *folder = ".";

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
            continue;

        folder = argv[i];
        break;
    }

    return ls(folder, &opt);
}