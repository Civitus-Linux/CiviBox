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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_MAX 1024
#define ARG_SIZE 64

enum { NONE, SINGLE, DOUBLE };

int parse(char *s, char **args)
{
    static char buf[INPUT_MAX];
    size_t pos = 0;
    int argc = 0, active = 0, quote = NONE;

    for (; *s; s++) {
        char c = *s;

        if (quote == NONE && (c == '\'' || c == '"')) {
            quote = c == '\'' ? SINGLE : DOUBLE;
            if (!active) {
                if (argc >= ARG_SIZE - 1) return -1;
                args[argc++] = &buf[pos];
                active = 1;
            }
        } else if ((quote == SINGLE && c == '\'') ||
                   (quote == DOUBLE && c == '"')) {
            quote = NONE;
        } else if (quote == NONE && (c == ' ' || c == '\t')) {
            if (active) {
                buf[pos++] = '\0';
                active = 0;
            }
        } else {
            if (!active) {
                if (argc >= ARG_SIZE - 1) return -1;
                args[argc++] = &buf[pos];
                active = 1;
            }

            if (pos >= sizeof(buf) - 1) return -1;
            buf[pos++] = c;
        }
    }

    if (quote != NONE) return -1;

    if (active) buf[pos++] = '\0';

    args[argc] = NULL;
    return argc;
}


int cvsh_cmd()
{
    char input[INPUT_MAX];

    while (1)
    {
        printf("$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) break;
        input[strcspn(input, "\n")] = '\0';

        if (input[0] == '\0') continue;

        char *args[ARG_SIZE];
        int argc = parse(input, args);

        if (argc < 0)
        {
            fprintf(stderr, "cvsh: invalid syntax\n");
            continue;
        }

        if (!argc) continue;

        if (strcmp(args[0], "exit") == 0) break;

        if (strcmp(args[0], "cd") == 0)
        {
            const char *path = argc > 1 ? args[1] : getenv("HOME");

            if (path == NULL || chdir(path) != 0) perror("cd");

            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            execvp(args[0], args);

            perror("cvsh");
            _exit(127);
        }
        else waitpid(pid, NULL, 0);
    }
    return 0;
}
