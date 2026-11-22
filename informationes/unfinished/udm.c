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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <shadow.h>
#include <crypt.h>
#include <grp.h>

#define LEFT  1000
#define RIGHT 1001
#define TAB   1002
#define ENTER 1003
#define BACK  1004

#define WIDTH  40
#define HEIGHT 11
#define INPUT  64

char *tests[] = {"test1", "test2"};
int total = sizeof(tests) / sizeof(*tests);
int selected, focus;

char username[INPUT], password[INPUT];
struct termios old;

void tui_init(void)
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

void tui_end(void)
{
  tcsetattr(0, TCSANOW, &old);
  printf("\033[0m\033[?25h");
  fflush(stdout);
}

void cursor(int x, int y)
{
  printf("\033[%d;%dH", y, x);
}

void clear(void)
{
  printf("\033[2J\033[H");
}

int getkey(void)
{
  char c, seq[2];

  if (read(0, &c, 1) != 1) return -1;

  if (c == '\033') {
    if (read(0, seq, 2) != 2) return -1;
    if (seq[0] != '[') return -1;
    if (seq[1] == 'D') return LEFT;
    if (seq[1] == 'C') return RIGHT;
    return -1;
  }

  if (c == '\t') return TAB;
  if (c == '\n' || c == '\r') return ENTER;
  if (c == 127 || c == 8) return BACK;

  return (unsigned char)c;
}

void box(int x, int y)
{
  cursor(x, y);
  printf("╭");
  for (int i = 0; i < WIDTH - 2; i++) printf("─");
  printf("╮");

  for (int i = 1; i < HEIGHT - 1; i++) {
    cursor(x, y + i);
    printf("│");
    cursor(x + WIDTH - 1, y + i);
    printf("│");
  }

  cursor(x, y + HEIGHT - 1);
  printf("╰");
  for (int i = 0; i < WIDTH - 2; i++) printf("─");
  printf("╯");
}

void text(int x, int y, const char *s, int active)
{
  int pos = x + (WIDTH - strlen(s)) / 2;

  cursor(pos, y);
  if (active) printf("\033[7m");
  printf("%s", s);
  if (active) printf("\033[0m");
}

void input(char *buffer, int key)
{
  int len = strlen(buffer);

  if (key == BACK) {
    if (len > 0) buffer[len - 1] = '\0';
  } else if (key >= 32 && key <= 126 && len < INPUT - 1) {
    buffer[len] = key;
    buffer[len + 1] = '\0';
  }
}

void draw(void)
{
  struct winsize ws;

  if (ioctl(1, TIOCGWINSZ, &ws) == -1) return;

  int x = (ws.ws_col - WIDTH) / 2 + 1;
  int y = (ws.ws_row - HEIGHT) / 2 + 1;

  if (x < 1) x = 1;
  if (y < 1) y = 1;

  char option[128], hidden[INPUT];

  snprintf(option, sizeof(option), "< %s >", tests[selected]);

  int pass_len = strlen(password);
  memset(hidden, '*', pass_len);
  hidden[pass_len] = '\0';

  clear();
  box(x, y);

  text(x, y + 2, option, focus == 0);
  text(x, y + 4, username[0] ? username : "Username", focus == 1);
  text(x, y + 5, password[0] ? hidden : "Password", focus == 2);
  text(x, y + 7, "[ OK ]", focus == 3);

  if (focus == 1 || focus == 2) {
    char *buffer = focus == 1 ? username : password;
    const char *placeholder = focus == 1 ? "Username" : "Password";
    int len = strlen(buffer);
    int display_len = len ? len : strlen(placeholder);
    int pos = x + (WIDTH - display_len) / 2;
    int line = y + (focus == 1 ? 4 : 5);

    cursor(pos + display_len, line);
    printf("\033[?25h");
  } else {
    printf("\033[?25l");
  }

  fflush(stdout);
}

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

void bye(void)
{
  tui_end();
  clear();

  struct passwd *user;
  user = find_user(username);

  if (!user) return;
  if (!user->pw_name || !user->pw_dir || !user->pw_shell || initgroups(user->pw_name, user->pw_gid) || setgid(user->pw_gid) < 0 || setuid(user->pw_uid) < 0 || chdir(user->pw_dir) < 0) return;

    clearenv();

    setenv("HOME", user->pw_dir, 1);
    setenv("USER", user->pw_name, 1);
    setenv("LOGNAME", user->pw_name, 1);
    setenv("SHELL", user->pw_shell, 1);
    setenv("PATH", "/usr/local/bin:/usr/bin:/bin", 1);

    char *base = strrchr(user->pw_shell, '/');
    base = base ? base + 1 : user->pw_shell;

    char *argv0 = malloc(strlen(base) + 2);
    if (!argv0) return;

    sprintf(argv0, "-%s", base);

    char *argv[] = {
        argv0,
        NULL
c    };

    execve(user->pw_shell, argv, environ);

    perror("execve");
}

int udm_cmd(void)
{
  tui_init();

  while (1) {
    draw();

    int key = getkey();

    if (focus == 0 && key == LEFT) selected = (selected - 1 + total) % total;
    if (focus == 0 && key == RIGHT) selected = (selected + 1) % total;

    if (focus == 1) input(username, key);
    if (focus == 2) input(password, key);

    if (key == TAB) focus = (focus + 1) % 4;

    if (key == ENTER) {
      if (focus == 3) {
        bye();
        return 0;
      }

      focus = (focus + 1) % 4;
    }
  }

  tui_end();
  return 0;
}

