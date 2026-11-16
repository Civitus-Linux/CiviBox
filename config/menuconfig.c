/*
 * CiviBox - Minimal userspace toolkit for Linux
 *
 * Copyright (C) 2026 Pietro Carlini
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2
 * of the License, or (at your option) any later version.
 */

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>

#define CONFIG_FILE ".config"
#define INFORMATIONES_DIR "informationes"

#define MAX_COMMANDS 512
#define MAX_CATEGORIES 64
#define MAX_NAME 128
#define MAX_CONFIG 256

struct command
{
    char category[MAX_NAME];
    char name[MAX_NAME];
    char config[MAX_CONFIG];
    int enabled;
};

struct config
{
    int compile_static;
};

static struct command commands[MAX_COMMANDS];
static int commands_total;

static char categories[MAX_CATEGORIES][MAX_NAME];
static int categories_total;

static struct config cfg;

static int compare_strings(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

static void make_config_name(char *buffer,
                             size_t size,
                             const char *category,
                             const char *command)
{
    size_t pos = 0;

    if (size == 0)
        return;

    pos += (size_t)snprintf(buffer, size, "CONFIG_");

    const char *parts[] = { category, command };

    for (int p = 0; p < 2 && pos < size - 1; p++)
    {
        if (p == 1)
        {
            buffer[pos++] = '_';
            if (pos >= size - 1)
                break;
        }

        for (size_t i = 0;
             parts[p][i] && pos < size - 1;
             i++)
        {
            unsigned char c = (unsigned char)parts[p][i];

            if (isalnum(c))
                buffer[pos++] = (char)toupper(c);
            else
                buffer[pos++] = '_';
        }
    }

    buffer[pos] = '\0';
}

static int is_c_file(const char *name)
{
    size_t len = strlen(name);

    return len > 2 &&
           strcmp(name + len - 2, ".c") == 0;
}

static int category_exists(const char *name)
{
    for (int i = 0; i < categories_total; i++)
    {
        if (strcmp(categories[i], name) == 0)
            return 1;
    }

    return 0;
}

static void discover_commands(void)
{
    DIR *root = opendir(INFORMATIONES_DIR);

    if (!root)
        return;

    struct dirent *entry;

    while ((entry = readdir(root)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (categories_total >= MAX_CATEGORIES)
            break;

        char path[PATH_MAX];

        snprintf(path, sizeof(path),
                 "%s/%s",
                 INFORMATIONES_DIR,
                 entry->d_name);

        DIR *directory = opendir(path);

        if (!directory)
            continue;

        if (!category_exists(entry->d_name))
        {
            snprintf(categories[categories_total],
                     sizeof(categories[0]),
                     "%s",
                     entry->d_name);

            categories_total++;
        }

        struct dirent *file;

        while ((file = readdir(directory)) != NULL)
        {
            if (!is_c_file(file->d_name))
                continue;

            if (commands_total >= MAX_COMMANDS)
                break;

            struct command *cmd = &commands[commands_total];

            snprintf(cmd->category,
                     sizeof(cmd->category),
                     "%s",
                     entry->d_name);

            size_t len = strlen(file->d_name) - 2;

            if (len >= sizeof(cmd->name))
                continue;

            memcpy(cmd->name, file->d_name, len);
            cmd->name[len] = '\0';

            make_config_name(cmd->config,
                             sizeof(cmd->config),
                             cmd->category,
                             cmd->name);

            cmd->enabled = 1;

            commands_total++;
        }

        closedir(directory);
    }

    closedir(root);

    qsort(categories,
          (size_t)categories_total,
          sizeof(categories[0]),
          compare_strings);

    for (int i = 0; i < commands_total; i++)
    {
        for (int j = i + 1; j < commands_total; j++)
        {
            int cmp = strcmp(commands[i].category,
                             commands[j].category);

            if (cmp > 0 ||
                (cmp == 0 &&
                 strcmp(commands[i].name,
                        commands[j].name) > 0))
            {
                struct command tmp = commands[i];
                commands[i] = commands[j];
                commands[j] = tmp;
            }
        }
    }
}

static void load_config(void)
{
    FILE *file = fopen(CONFIG_FILE, "r");

    cfg.compile_static = 1;

    if (!file)
    {
        for (int i = 0; i < commands_total; i++)
            commands[i].enabled = 1;

        return;
    }

    for (int i = 0; i < commands_total; i++)
        commands[i].enabled = 0;

    char line[512];

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line,
                    "CONFIG_COMPILE_STATIC=y",
                    23) == 0)
        {
            cfg.compile_static = 1;
            continue;
        }

        if (strncmp(line,
                    "CONFIG_COMPILE_STATIC=n",
                    23) == 0)
        {
            cfg.compile_static = 0;
            continue;
        }

        for (int i = 0; i < commands_total; i++)
        {
            char enabled[MAX_CONFIG + 8];
            char disabled[MAX_CONFIG + 32];

            snprintf(enabled,
                     sizeof(enabled),
                     "%s=y",
                     commands[i].config);

            snprintf(disabled,
                     sizeof(disabled),
                     "# %s is not set",
                     commands[i].config);

            if (strncmp(line, enabled, strlen(enabled)) == 0)
            {
                commands[i].enabled = 1;
                break;
            }

            if (strncmp(line, disabled, strlen(disabled)) == 0)
            {
                commands[i].enabled = 0;
                break;
            }
        }
    }

    fclose(file);
}

static void save_config(void)
{
    FILE *file = fopen(CONFIG_FILE, "w");

    if (!file)
        return;

    fprintf(file,
            "#\n"
            "# CiviBox configuration\n"
            "# Automatically generated by menuconfig\n"
            "#\n\n");

    fprintf(file,
            "CONFIG_COMPILE_STATIC=%c\n\n",
            cfg.compile_static ? 'y' : 'n');

    for (int i = 0; i < commands_total; i++)
    {
        if (commands[i].enabled)
        {
            fprintf(file,
                    "%s=y\n",
                    commands[i].config);
        }
        else
        {
            fprintf(file,
                    "# %s is not set\n",
                    commands[i].config);
        }
    }

    fclose(file);
}

static void draw_header(const char *title)
{
    attron(COLOR_PAIR(1) | A_BOLD);

    mvhline(0, 0, ' ', COLS);

    int x = (COLS - (int)strlen(title)) / 2;

    if (x < 0)
        x = 0;

    mvprintw(0, x, "%s", title);

    attroff(COLOR_PAIR(1) | A_BOLD);
}

static void draw_footer(void)
{
    int row = LINES - 2;

    attron(COLOR_PAIR(1));

    mvhline(row, 0, ' ', COLS);
    mvhline(row + 1, 0, ' ', COLS);

    mvprintw(row, 2,
             "<Enter> Select   <Space> Toggle   <Esc> Back");

    mvprintw(row + 1, 2,
             "<S> Save         <Q> Quit");

    attroff(COLOR_PAIR(1));
}

static void draw_item(int row,
                      int selected,
                      const char *text)
{
    if (row < 1 || row >= LINES - 2)
        return;

    if (selected)
        attron(COLOR_PAIR(2) | A_BOLD);
    else
        attron(COLOR_PAIR(1));

    mvhline(row, 0, ' ', COLS);

    mvprintw(row, 3, "%s", text);

    if (selected)
        attroff(COLOR_PAIR(2) | A_BOLD);
    else
        attroff(COLOR_PAIR(1));
}

static int category_command_count(const char *category)
{
    int count = 0;

    for (int i = 0; i < commands_total; i++)
    {
        if (strcmp(commands[i].category, category) == 0)
            count++;
    }

    return count;
}

static int category_command_index(const char *category,
                                  int position)
{
    for (int i = 0; i < commands_total; i++)
    {
        if (strcmp(commands[i].category, category) == 0)
        {
            if (position == 0)
                return i;

            position--;
        }
    }

    return -1;
}

static void compile_menu(void)
{
    int running = 1;

    while (running)
    {
        erase();

        draw_header("Compile settings");

        char text[128];

        snprintf(text,
                 sizeof(text),
                 "[%c] COMPILE_STATIC",
                 cfg.compile_static ? '*' : ' ');

        draw_item(3, 1, text);

        draw_footer();
        refresh();

        int key = getch();

        switch (key)
        {
            case ' ':
                cfg.compile_static = !cfg.compile_static;
                break;

            case 's':
            case 'S':
                save_config();
                break;

            case 27:
            case 'q':
            case 'Q':
                running = 0;
                break;
        }
    }
}

static void commands_menu(const char *category)
{
    int selected = 0;
    int running = 1;

    int count = category_command_count(category);

    if (count == 0)
        return;

    while (running)
    {
        erase();

        char title[160];

        snprintf(title,
                 sizeof(title),
                 "Commands (%s)",
                 category);

        draw_header(title);

        int visible = LINES - 7;

        if (visible < 1)
            visible = 1;

        int start = 0;

        if (selected >= visible)
            start = selected - visible + 1;

        for (int i = 0;
             i < visible && start + i < count;
             i++)
        {
            int index = category_command_index(
                category,
                start + i
            );

            if (index < 0)
                continue;

            char text[256];

            snprintf(text,
                     sizeof(text),
                     "[%c] %s",
                     commands[index].enabled ? '*' : ' ',
                     commands[index].name);

            draw_item(3 + i,
                      selected == start + i,
                      text);
        }

        draw_footer();
        refresh();

        int key = getch();

        switch (key)
        {
            case KEY_UP:
                if (selected > 0)
                    selected--;
                break;

            case KEY_DOWN:
                if (selected < count - 1)
                    selected++;
                break;

            case KEY_NPAGE:
                selected += visible;

                if (selected >= count)
                    selected = count - 1;
                break;

            case KEY_PPAGE:
                selected -= visible;

                if (selected < 0)
                    selected = 0;
                break;

            case KEY_HOME:
                selected = 0;
                break;

            case KEY_END:
                selected = count - 1;
                break;

            case ' ':
            {
                int index = category_command_index(
                    category,
                    selected
                );

                if (index >= 0)
                    commands[index].enabled =
                        !commands[index].enabled;

                break;
            }

            case 's':
            case 'S':
                save_config();
                break;

            case 27:
            case 'q':
            case 'Q':
                running = 0;
                break;
        }
    }
}

static void main_menu(void)
{
    int selected = 0;
    int running = 1;

    int count = categories_total + 1;

    while (running)
    {
        erase();

        draw_header("CiviBox Configuration");

        int visible = LINES - 7;

        if (visible < 1)
            visible = 1;

        int start = 0;

        if (selected >= visible)
            start = selected - visible + 1;

        for (int i = 0;
             i < visible && start + i < count;
             i++)
        {
            int index = start + i;

            char text[160];

            if (index == 0)
            {
                snprintf(text,
                         sizeof(text),
                         "Compile settings  --->");
            }
            else
            {
                snprintf(text,
                         sizeof(text),
                         "Commands (%s)  --->",
                         categories[index - 1]);
            }

            draw_item(3 + i,
                      selected == index,
                      text);
        }

        draw_footer();
        refresh();

        int key = getch();

        switch (key)
        {
            case KEY_UP:
                if (selected > 0)
                    selected--;
                break;

            case KEY_DOWN:
                if (selected < count - 1)
                    selected++;
                break;

            case KEY_NPAGE:
                selected += visible;

                if (selected >= count)
                    selected = count - 1;
                break;

            case KEY_PPAGE:
                selected -= visible;

                if (selected < 0)
                    selected = 0;
                break;

            case KEY_HOME:
                selected = 0;
                break;

            case KEY_END:
                selected = count - 1;
                break;

            case '\n':
            case KEY_ENTER:
                if (selected == 0)
                {
                    compile_menu();
                }
                else if (selected > 0)
                {
                    commands_menu(categories[selected - 1]);
                }
                break;

            case 's':
            case 'S':
                save_config();
                break;

            case 27:
            case 'q':
            case 'Q':
                running = 0;
                break;
        }
    }
}

int main(void)
{
    discover_commands();
    load_config();

    initscr();

    if (has_colors())
    {
        start_color();

        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        init_pair(2, COLOR_BLUE, COLOR_WHITE);

        bkgd(COLOR_PAIR(1));
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    curs_set(0);

    main_menu();

    endwin();

    return 0;
}
