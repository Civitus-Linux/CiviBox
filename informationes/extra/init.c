/* CiviBox - Minimal userspace toolkit for Linux
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


#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int init_cmd(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (getpid() != 1) {
        fprintf(stderr, "init: must run as PID 1\n");
        return 1;
    }

    if (access("/bin/sh", X_OK) != 0) {
        perror("init: /bin/sh");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("init: fork");
        return 1;
    }

    if (pid == 0) {
        if (access("/etc/rc", F_OK) == 0)
            execl("/bin/sh", "cvsh", "/etc/rc", (char *)NULL);
        else
            execl("/bin/sh", "cvsh", (char *)NULL);

        perror("init: exec /bin/sh");
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("init: waitpid");
        return 1;
    }

    return 0;
}
