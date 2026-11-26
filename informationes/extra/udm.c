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

#define _DEFAULT_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include "auth/auth.h"

enum { LEFT = 1000, RIGHT, TAB, ENTER, BACK, QUIT };

#define WIDTH       40
#define HEIGHT      11
#define INPUT       64
#define DESKTOP_MAX 128
#define UDM_DIR     "/etc/udm"

static char message[INPUT], username[INPUT], password[INPUT];
static char *desktops[DESKTOP_MAX];
static int ndesktops, selected, focus;
static struct termios old;

static void cursor(int x, int y) { printf("\033[%d;%dH", y, x); }
static void clear(void) { printf("\033[2J\033[H"); }

static void tui_init(void)
{
  struct termios raw;

  tcgetattr(0, &old);
  raw = old;
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;
  tcsetattr(0, TCSANOW, &raw);
  printf("\033[?25l");
}

static void tui_end(void)
{
  tcsetattr(0, TCSANOW, &old);
  printf("\033[0m\033[?25h");
  fflush(stdout);
}

static int getkey(void)
{
  unsigned char c, s[2];

  if (read(0, &c, 1) != 1) return QUIT;
  if (c == '\033') {
    if (read(0, s, 2) != 2 || s[0] != '[') return 0;
    return s[1] == 'D' ? LEFT : s[1] == 'C' ? RIGHT : 0;
  }
  if (c == '\t') return TAB;
  if (c == '\n' || c == '\r') return ENTER;
  if (c == 127 || c == 8) return BACK;
  return c;
}

static void box(int x, int y)
{
  int i;

  cursor(x, y);
  printf("╭");
  for (i = 0; i < WIDTH - 2; i++) printf("─");
  printf("╮");
  for (i = 1; i < HEIGHT - 1; i++) {
    cursor(x, y + i);
    printf("│");
    cursor(x + WIDTH - 1, y + i);
    printf("│");
  }
  cursor(x, y + HEIGHT - 1);
  printf("╰");
  for (i = 0; i < WIDTH - 2; i++) printf("─");
  printf("╯");
}

static void text(int x, int y, const char *s, int active)
{
  cursor(x + (WIDTH - (int)strlen(s)) / 2, y);
  printf(active ? "\033[7m%s\033[0m" : "%s", s);
}

static void input(char *buf, int key)
{
  size_t len = strlen(buf);

  message[0] = '\0';
  if (key == BACK && len) buf[len - 1] = '\0';
  else if (key >= 32 && key <= 126 && len < INPUT - 1) {
    buf[len] = key;
    buf[len + 1] = '\0';
  }
}

static void draw(void)
{
  struct winsize ws;
  char opt[128], hidden[INPUT], *buf;
  const char *hint;
  int x, y, len;

  if (ioctl(1, TIOCGWINSZ, &ws) == -1) return;
  x = (ws.ws_col - WIDTH) / 2 + 1;
  y = (ws.ws_row - HEIGHT) / 2 + 1;
  if (x < 1) x = 1;
  if (y < 1) y = 1;

  snprintf(opt, sizeof opt, "< %s >", desktops[selected]);
  len = strlen(password);
  memset(hidden, '*', len);
  hidden[len] = '\0';

  clear();
  box(x, y);
  text(x, y + 1, message, 0);
  text(x, y + 2, opt, focus == 0);
  text(x, y + 4, username[0] ? username : "Username", focus == 1);
  text(x, y + 5, password[0] ? hidden : "Password", focus == 2);
  text(x, y + 7, "[ OK ]", focus == 3);

  if (focus != 1 && focus != 2) printf("\033[?25l");
  else {
    buf = focus == 1 ? username : password;
    hint = focus == 1 ? "Username" : "Password";
    len = *buf ? (int)strlen(buf) : (int)strlen(hint);
    cursor(x + (WIDTH - len) / 2 + len, y + 3 + focus);
    printf("\033[?25h");
  }
  fflush(stdout);
}

static void scan_desktops(void)
{
  struct dirent *e;
  DIR *d = opendir(UDM_DIR);

  if (!d) return;
  while (ndesktops < DESKTOP_MAX && (e = readdir(d))) {
    if (e->d_name[0] == '.') continue;
    if (!(desktops[ndesktops] = strdup(e->d_name))) break;
    ndesktops++;
  }
  closedir(d);
}

static int session(const struct passwd *pw)
{
  char path[512], log[512];
  int fd, st;
  pid_t pid = fork();

  if (pid < 0) return -1;
  if (pid > 0) {
    waitpid(pid, &st, 0);
    fchown(0, 0, 0);
    fchmod(0, 0600);
    return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
  }

  if (setsid() < 0) _exit(1);
  ioctl(0, TIOCSCTTY, 1);
  fchown(0, pw->pw_uid, pw->pw_gid);
  fchmod(0, 0620);

  if (initgroups(pw->pw_name, pw->pw_gid) < 0 ||
      setgid(pw->pw_gid) < 0 || setuid(pw->pw_uid) < 0 ||
      chdir(pw->pw_dir) < 0) _exit(1);

  clearenv();
  setenv("HOME", pw->pw_dir, 1);
  setenv("USER", pw->pw_name, 1);
  setenv("LOGNAME", pw->pw_name, 1);
  setenv("SHELL", pw->pw_shell, 1);
  setenv("TERM", "linux", 1);
  setenv("PATH", "/usr/local/bin:/usr/bin:/bin", 1);

  snprintf(log, sizeof log, "%s/.udm.log", pw->pw_dir);
  fd = open(log, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0600);
  if (fd >= 0) { dup2(fd, 1); dup2(fd, 2); }

  snprintf(path, sizeof path, UDM_DIR "/%s", desktops[selected]);
  execl("/bin/sh", "sh", path, (char *)NULL);
  _exit(127);
}

int udm_cmd(void)
{
  struct passwd *pw;
  int key, r;

  if (geteuid() != 0) { puts("Are you root?"); return 1; }

  scan_desktops();
  if (!ndesktops) { fputs("udm: no desktops in " UDM_DIR "\n", stderr); return 1; }
  freopen("/var/log/udm.log", "a", stderr);

  tui_init();
  for (;;) {
    draw();
    if ((key = getkey()) == QUIT) break;

    if (focus == 0 && key == LEFT) selected = (selected + ndesktops - 1) % ndesktops;
    if (focus == 0 && key == RIGHT) selected = (selected + 1) % ndesktops;
    if (focus == 1) input(username, key);
    if (focus == 2) input(password, key);
    if (key == TAB) focus = (focus + 1) % 4;
    if (key != ENTER) continue;

    if (focus != 3) { focus = (focus + 1) % 4; continue; }

    r = auth_check(username, password);
    explicit_bzero(password, sizeof password);
    if (r == AUTH_OK && (pw = getpwnam(username))) {
      clear();
      tui_end();
      if (session(pw)) fputs("udm: session failed, see ~/.udm.log\n", stderr);
      return 0;
    }
    snprintf(message, sizeof message, "%s", r == AUTH_DENIED ? "Invalid login" : "Authentication error");
    focus = 2;
  }

  tui_end();
  return 1;
}
