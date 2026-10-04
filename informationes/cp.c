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
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

static int i;

static void usage(void) {
  xwrite("cp [OPTION] SOURCE... DIRECTORY\n");
  xwrite(" -f remove destination before copying");
  xwrite(" -i ask before overwrite");
  xwrite(" -r copy directories recursively");
}

static int ask(const char * file) {
  char answer[8];

  xwrite("cp: overwrite '%s'? (y/n) [n] ", file);

  if (fgets(answer, sizeof(answer), stdin) == NULL)
    return 0;

  return answer[0] == 'y' || answer[0] == 'Y';
}

static int cpfile(const char * to,
  const char * from) {
  int fd_to;
  int fd_from;

  char buf[4096];
  ssize_t nread;

  fd_from = open(from, O_RDONLY);

  if (fd_from < 0)
    return 1;

  if (i) {
    if (access(to, F_OK) == 0) {
      if (!ask(to)) {
        close(fd_from);
        return 0;
      }
    }

    fd_to = open(to,
      O_WRONLY | O_CREAT | O_TRUNC,
      0666);
  } else {
    fd_to = open(to,
      O_WRONLY | O_CREAT | O_EXCL,
      0666);
  }

  if (fd_to < 0) {
    close(fd_from);
    return 1;
  }

  while ((nread = read(fd_from, buf, sizeof(buf))) > 0) {
    ssize_t written = 0;

    while (written < nread) {
      ssize_t nwrite;

      nwrite = write(fd_to,
        buf + written,
        nread - written);

      if (nwrite < 0) {
        close(fd_from);
        close(fd_to);
        return 1;
      }

      written += nwrite;
    }
  }

  close(fd_from);
  close(fd_to);

  return nread == 0 ? 0 : 1;
}

static int cpdir(const char * to,
  const char * from) {
  DIR * dir;
  struct dirent * entry;

  if (mkdir(to, 0755) != 0 && errno != EEXIST)
    return 1;

  dir = opendir(from);

  if (!dir)
    return 1;

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry -> d_name, ".") == 0 ||
      strcmp(entry -> d_name, "..") == 0) {
      continue;
    }

    char src[4096];
    char dst[4096];

    snprintf(src,
      sizeof(src),
      "%s/%s",
      from,
      entry -> d_name);

    snprintf(dst,
      sizeof(dst),
      "%s/%s",
      to,
      entry -> d_name);

    struct stat st;

    if (stat(src, & st) != 0) {
      closedir(dir);
      return 1;
    }

    if (S_ISDIR(st.st_mode)) {
      if (cpdir(dst, src) != 0) {
        closedir(dir);
        return 1;
      }
    } else {
      if (cpfile(dst, src) != 0) {
        closedir(dir);
        return 1;
      }
    }
  }

  closedir(dir);

  return 0;
}

int cp_cmd(int argc, char ** argv) {
  if (argc < 3) {
    usage();
    return 1;
  }

  int f = getArg(argc, argv, "-f");
  i = getArg(argc, argv, "-i");
  int r = getArg(argc, argv, "-r");

  const char * from = argv[argc - 2];
  const char * to = argv[argc - 1];

  struct stat st;

  if (stat(from, & st) != 0) {
    xwrite("cp: cannot stat '%s'\n", from);
    return 1;
  }

  if (S_ISDIR(st.st_mode)) {
    if (!r) {
      xwrite("cp: '%s' is a directory\n", from);
      return 1;
    }

    if (f) {
      if (unlink(to) != 0 && errno != ENOENT) {
        xwrite("cp: cannot remove '%s'\n", to);
        return 1;
      }
    }

    return cpdir(to, from);
  }

  if (f) {
    if (unlink(to) != 0 && errno != ENOENT) {
      xwrite("cp: cannot remove '%s'\n", to);
      return 1;
    }
  }

  if (cpfile(to, from) != 0) {
    xwrite("cp: cannot copy '%s' to '%s'\n",
      from,
      to);

    return 1;
  }

  return 0;
}