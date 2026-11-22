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
#include <unistd.h>

int whoami_cmd(void)
{
    struct passwd *user = getpwuid(geteuid());

    if (user == NULL) return 1;
    puts(user->pw_name);
    return 0;
}
