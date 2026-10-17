#!/bin/sh
set -eu
cd "$(dirname "$0")"
for compiler in gcc clang; do
    command -v "$compiler" >/dev/null 2>&1 || continue
    binary="$(mktemp)"
    trap 'rm -f "$binary"' EXIT HUP INT TERM
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        gpd_commands.c gpd_commands_test.c -o "$binary"
    "$binary"
    rm -f "$binary"
    trap - EXIT HUP INT TERM
done
