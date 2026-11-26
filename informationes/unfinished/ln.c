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

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"

static void usage(void)
{
    xwrite("ln [-fs] [-L|-P] SOURCE... TARGET\n");
    xwrite("    -s                  Create symbolic links instead of hard links");
    xwrite("    -f                  Remove existing destination files");
    xwrite("    -L                  Hard link to the file a symlink points to");
    xwrite("    -P                  Hard link to the symlink itself");
}

static int link_one(const char *src, const char *dst, int sym, int force, int follow)
{
    struct stat a, b;

    if (!lstat(dst, &b)) {
        if (!force) { fprintf(stderr, "ln: %s: File exists\n", dst); return 1; }
        if (!lstat(src, &a) && a.st_dev == b.st_dev && a.st_ino == b.st_ino) { fprintf(stderr, "ln: %s and %s are the same file\n", src, dst); return 1; }
        if (unlink(dst) < 0) { perror(dst); return 1; }
    }
    if (sym ? symlink(src, dst) : linkat(AT_FDCWD, src, AT_FDCWD, dst, follow ? AT_SYMLINK_FOLLOW : 0)) { perror(dst); return 1; }
    return 0;
}

int ln_cmd(int argc, char **argv)
{
    if (argc == 1)
    {
        usage();
        return 0;
    }

    int sym = 0, force = 0, follow = 0, i, j;

    for (i = 1; i < argc && argv[i][0] == '-' && argv[i][1]; i++) {
        if (!strcmp(argv[i], "--")) { i++; break; }
            for (j = 1; argv[i][j]; j++) switch (argv[i][j]) {
                case 's': sym = 1; break;
                case 'f': force = 1; break;
                case 'L': follow = 1; break;
                case 'P': follow = 0; break;
            default: usage(); return 1;
        }
    }

    int n = argc - i, rc = 0;
    const char *dir = argv[argc - 1], *base;
    char path[PATH_MAX];
    struct stat st;

    if (n < 2) { usage(); return 1; }
    if (stat(dir, &st) || !S_ISDIR(st.st_mode)) {
        if (n > 2) { fprintf(stderr, "ln: %s: not a directory\n", dir); return 1; }
        return link_one(argv[i], dir, sym, force, follow);
    }
    for (; i < argc - 1; i++) {
        base = strrchr(argv[i], '/');
        base = base ? base + 1 : argv[i];
        snprintf(path, sizeof path, "%s/%s", dir, base);
        rc |= link_one(argv[i], path, sym, force, follow);
    }
    return rc;
}
