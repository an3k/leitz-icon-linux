#!/bin/sh
set -eu
cd "$(dirname "$0")"
for cc in gcc clang; do
    command -v "$cc" >/dev/null 2>&1 || continue
    binary="$(mktemp)"
    trap 'rm -f "$binary"' EXIT HUP INT TERM
    "$cc" -std=c11 -Wall -Wextra -Werror -pedantic -O2 leitz_ui_recovered.c leitz_ui_recovered_test.c -o "$binary"
    "$binary"
    rm -f "$binary"
    trap - EXIT HUP INT TERM
done
