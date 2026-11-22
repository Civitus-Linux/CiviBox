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

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <shadow.h>
#include <crypt.h>
#include <grp.h>

#include "common.h"

extern char **environ;

struct passwd *find_user(char *username)
{
    return getpwnam(username);
}

int check_pass(const char *user, const char *pass)
{
    struct spwd *sp;
    char *hash;

    sp = getspnam(user);
    if (!sp || !sp->sp_pwdp || sp->sp_pwdp[0] == '!' || sp->sp_pwdp[0] == '*') return 0;
    hash = crypt(pass, sp->sp_pwdp);
    if (!hash) return 0;

    return strcmp(hash, sp->sp_pwdp) == 0;
}

int login_cmd(void)
{
    struct passwd *user;
    for (;;) {
        char username[256], password[256];
        struct termios original, no_echo;

        printf("Username: ");
        fflush(stdout);
        if (fgets(username, sizeof username, stdin) == NULL) return 1;
        username[strcspn(username, "\n")] = '\0';

        printf("Password: ");
        fflush(stdout);

        if (tcgetattr(STDIN_FILENO, &original) == -1) {
            perror("tcgetattr");
            return 1;
        }

        no_echo = original;
        no_echo.c_lflag &= (tcflag_t)~ECHO;

        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &no_echo) == -1) {
            perror("tcsetattr");
            return 1;
        }

        int ok = fgets(password, sizeof password, stdin) != NULL;

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
        putchar('\n');

        if (!ok) return 1;
        password[strcspn(password, "\n")] = '\0';

        if (check_pass(username, password))
        {
            user = find_user(username);
            break;
        }
        xwrite("Login incorrect");
    }

    if (!user) return -1;
    if (!user->pw_name || !user->pw_dir || !user->pw_shell || initgroups(user->pw_name, user->pw_gid) || setgid(user->pw_gid) < 0 || setuid(user->pw_uid) < 0 || chdir(user->pw_dir) < 0) return -1;

    clearenv();

    setenv("HOME", user->pw_dir, 1);
    setenv("USER", user->pw_name, 1);
    setenv("LOGNAME", user->pw_name, 1);
    setenv("SHELL", user->pw_shell, 1);
    setenv("PATH", "/usr/local/bin:/usr/bin:/bin", 1);

    char *base = strrchr(user->pw_shell, '/');
    base = base ? base + 1 : user->pw_shell;

    char *argv0 = malloc(strlen(base) + 2);
    if (!argv0)
        return -1;

    sprintf(argv0, "-%s", base);

    char *argv[] = {
        argv0,
        NULL
    };

    execve(user->pw_shell, argv, environ);

    perror("execve");

    return 0;
}
