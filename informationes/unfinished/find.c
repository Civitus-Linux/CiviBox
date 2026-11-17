#include <stdio.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include "../../include/common.h"

static void usage()
{
    puts("Usage: find [PATH...] [EXPRESSION]");
    puts("");
    puts("Search for files and directories.");
    puts("");
    puts("    -name PATTERN       Match filename");
    puts("    -iname PATTERN      Match filename, ignoring case");
    puts("    -type TYPE          Filter by type (f, d)");
    puts("    -maxdepth N         Limit directory depth");
    puts("    -mindepth N         Set minimum directory depth");
    puts("    -print              Print matching paths");
    puts("    -delete             Delete matching entries");
    puts("    -empty              Match empty files or directories");
}

int find(const char *path, const char *pattern)
{
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror(path);
        return 1;
    }


    struct dirent *entry;
    struct stat st;
    char fullpath[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        if (lstat(fullpath, &st) == -1) continue;
        if (pattern == NULL || fnmatch(pattern, entry->d_name, 0) == 0) printf("%s\n", fullpath);

        if (S_ISDIR(st.st_mode)) {
            find(fullpath, pattern);
        }
    }

    closedir(dir);
    return 0;
}

int find_cmd(int argc, char *argv[])
{
    if (argc < 2) {
        usage();
        return 1;
    }

    const char *name = getArg(argc, argv, "-name", 1);
    const char *path = (argv[1][0] == '-') ? "." : argv[1];

    return find(path, name == (char *)1 ? NULL : name);
}
