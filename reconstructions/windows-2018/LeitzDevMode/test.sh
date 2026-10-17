#!/bin/sh
set -eu
cd "$(dirname "$0")"
for cc in gcc clang; do
    command -v "$cc" >/dev/null 2>&1 || continue
    out="$(mktemp)"
    trap 'rm -f "$out"' EXIT HUP INT TERM
    "$cc" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
        oem_devmode.c oem_devmode_test.c -o "$out"
    "$out"
    rm -f "$out"
    trap - EXIT HUP INT TERM
done
