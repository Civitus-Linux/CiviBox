#!/bin/bash

set -e

mkdir -p etc

{
    echo '#ifndef CIVIBOX_COMMANDS'
    echo '#define CIVIBOX_COMMANDS'
    echo ''

    # Commands
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

    # Extra Commands
    echo 'static const char *extracommand_list[] ='
    echo '{'

    for f in ./extra/*.c; do
        [ -f "$f" ] || continue
        base="$(basename "$f" .c)"
        echo "    \"$base\","
    done

    echo '    NULL'
    echo '};'
    echo ''

    echo '#endif'

} > ./etc/commands.c
