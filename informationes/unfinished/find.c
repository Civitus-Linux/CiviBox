#include <stdio.h>
#include <dirent.h>
#include <fnmatch.h>
#include <ctype.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include "../../include/common.h"

static void usage(void)
{
    puts("Usage: find [PATH...] [EXPRESSION]");
    puts("");
    puts("Search for files and directories.");
    puts("");
    puts("    -name PATTERN       Match filename");
    puts("    -iname PATTERN      Match filename, ignoring case");
    puts("    -type TYPE          Filter by type (f, d, l, b, c, p, s)");
    puts("    -maxdepth N         Limit directory depth");
    puts("    -mindepth N         Set minimum directory depth");
    puts("    -print              Print matching paths");
    puts("    -delete             Delete matching entries");
    puts("    -empty              Match empty files or directories");
}

int match_type(const char *path, char type)
{
    struct stat st;
    if (lstat(path, &st) == -1) return 0;

    switch (type)
    {
        case 'f': return S_ISREG(st.st_mode);
        case 'd': return S_ISDIR(st.st_mode);
        case 'l': return S_ISLNK(st.st_mode);
        case 'b': return S_ISBLK(st.st_mode);
        case 'c': return S_ISCHR(st.st_mode);
        case 'p': return S_ISFIFO(st.st_mode);
        case 's': return S_ISSOCK(st.st_mode);
        default: return -1;
    }
}

int fm_case(const char *pattern, const char *string)
{
    char p[PATH_MAX];
    char s[PATH_MAX];

    size_t i;

    for (i = 0; pattern[i] && i < sizeof(p) - 1; i++)
        p[i] = tolower((unsigned char)pattern[i]);
    p[i] = '\0';

    for (i = 0; string[i] && i < sizeof(s) - 1; i++)
        s[i] = tolower((unsigned char)string[i]);
    s[i] = '\0';

    return fnmatch(p, s, 0);
}

int find(const char *path, const char *pattern, int flags, char type)
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
	    int matched = 1;

	    if (pattern != NULL) matched = fnmatch(pattern, entry->d_name, flags) == 0;

	    if (matched)
        {
            if (type == '\0' || match_type(fullpath, type) == 1) printf("%s\n", fullpath);
        }

        if (S_ISDIR(st.st_mode)) {
            find(fullpath, pattern, flags, type);
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
    const char *iname = getArg(argc, argv, "-iname", 1);
    const char *type_arg = getArg(argc, argv, "-type", 1);

    const char *pattern = name ? name : iname;
    int flags = iname ? FNM_CASEFOLD : 0;
    const char *path = (argv[1][0] == '-') ? "." : argv[1];
    char type = '\0';

    if (type_arg != NULL) {
        if (type_arg == (char *)1 || type_arg[1] != '\0') {
            fprintf(stderr, "find: invalid argument for -type\n");
            return 1;
        }

        type = type_arg[0];
        if (match_type(".", type) == -1) {
            fprintf(stderr, "find: invalid type '%c'\n", type);
            return 1;
        }
    }

    return find(path, pattern, flags, type);
}
