
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <crypt.h>
#include <time.h>
#include <errno.h>

#include "auth.h"

#define SHADOW_FILE "/etc/shadow"

static long long
field_number(const char *field)
{
    char *end;
    long long value;

    if (!field || !*field)
        return -1;

    errno = 0;
    value = strtoll(field, &end, 10);

    if (errno || *end)
        return -2;

    return value;
}

static int hash_equal(const char *a, const char *b)
{
    unsigned char diff = 0;

    while (*a && *b)
        diff |= (unsigned char)*a++ ^ (unsigned char)*b++;

    return diff == 0 && *a == '\0' && *b == '\0';
}

static int account_valid(char **fields)
{
    long long last_change;
    long long max_age;
    long long inactive;
    long long expiration;
    long long today;

    if (!fields[1] || !*fields[1])
        return 0;

    if (fields[1][0] == '!' || fields[1][0] == '*')
        return 0;

    last_change = field_number(fields[2]);
    max_age = field_number(fields[4]);
    inactive = field_number(fields[6]);
    expiration = field_number(fields[7]);

    if (last_change == -2 || max_age == -2 ||
        inactive == -2 || expiration == -2)
        return 0;

    today = (long long)(time(NULL) / 86400);

    if (last_change == 0)
        return 0;

    if (expiration >= 0 && today >= expiration)
        return 0;

    if (last_change >= 0 && max_age >= 0) {
        long long expires = last_change + max_age;

        if (today > expires)
            return 0;

        if (inactive >= 0 && today > expires + inactive)
            return 0;
    }

    return 1;
}

static int get_hash(const char *username, char *hash, size_t size)
{
    FILE *fp;
    char *line = NULL;
    size_t capacity = 0;
    ssize_t length;
    int found = 0;

    fp = fopen(SHADOW_FILE, "r");

    if (fp) {
        while ((length = getline(&line, &capacity, fp)) != -1) {
            char *fields[9];
            char *p;
            int i;

            if (length && line[length - 1] == '\n')
                line[length - 1] = '\0';

            fields[0] = line;
            p = line;

            for (i = 1; i < 9; i++) {
                p = strchr(p, ':');

                if (!p)
                    break;

                *p++ = '\0';
                fields[i] = p;
            }

            if (i != 9)
                continue;

            if (strcmp(fields[0], username) != 0)
                continue;

            if (!account_valid(fields))
                break;

            if (strlen(fields[1]) >= size)
                break;

            strcpy(hash, fields[1]);
            found = 1;
            break;
        }

        free(line);
        fclose(fp);

        return found ? 1 : 0;
    }

    return -1;
}

int auth_check(const char *username, const char *password)
{
    struct passwd *pw;
    char hash[512];
    char *encrypted;
    int result;

    if (!username || !password)
        return AUTH_ERROR;

    if (!(pw = getpwnam(username)))
        return AUTH_DENIED;

    result = get_hash(username, hash, sizeof(hash));

    if (result == -1) {
        if (!pw->pw_passwd || !*pw->pw_passwd ||
            pw->pw_passwd[0] == 'x' ||
            pw->pw_passwd[0] == '!' ||
            pw->pw_passwd[0] == '*')
            return AUTH_ERROR;

        if (strlen(pw->pw_passwd) >= sizeof(hash))
            return AUTH_ERROR;

        strcpy(hash, pw->pw_passwd);
    } else if (result == 0) {
        return AUTH_DENIED;
    }

    encrypted = crypt(password, hash);

    if (!encrypted)
        return AUTH_ERROR;

    return hash_equal(encrypted, hash) ? AUTH_OK : AUTH_DENIED;
}
