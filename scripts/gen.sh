#!/bin/bash
set -e

{
    echo '#ifndef CIVIBOX_COMMANDS'
    echo '#define CIVIBOX_COMMANDS'
    echo ''
    echo 'struct menu_command {'
    echo '    const char *category;'
    echo '    const char *name;'
    echo '    const char *config;'
    echo '};'
    echo ''
    echo 'static const struct menu_command command_list[] = {'

    for dir in informationes/*/; do
        [ -d "$dir" ] || continue

        category=$(basename "$dir")

        for f in "$dir"*.c; do
            [ -f "$f" ] || continue

            name=$(basename "$f" .c)
            key=$(printf '%s_%s' "$category" "$name" |
                  tr '[:lower:]' '[:upper:]')

            printf '    {"%s", "%s", "CONFIG_%s"},\n' \
                "$category" "$name" "$key"
        done
    done

    echo '    {NULL, NULL, NULL}'
    echo '};'
    echo ''
    echo '#endif'
} > ./etc/commands.c
