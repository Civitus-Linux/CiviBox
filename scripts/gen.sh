#!/bin/bash

{
    echo '#ifndef CIVIBOX_COMMANDS'
    echo '#define CIVIBOX_COMMANDS'
    echo ''
    echo 'static const char *command_list[] ='
    echo '{'

    for f in ./informationes/*.c; do
        [ -f "$f" ] || continue

        base="$(basename "$f" .c)"

        echo "    \"$base\","
    done

    echo '    NULL'
    echo '};'
    echo ''
    echo '#endif'
} > ./etc/commands.c