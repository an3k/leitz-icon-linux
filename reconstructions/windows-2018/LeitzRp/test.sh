#!/bin/sh
set -eu
cd "$(dirname "$0")"
for compiler in gcc clang; do
    command -v "$compiler" >/dev/null 2>&1 || continue
    binary="$(mktemp)"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        filter_graphics_fragments.c filter_graphics_fragments_test.c -o "$binary"
    "$binary"
    rm -f "$binary"
done
