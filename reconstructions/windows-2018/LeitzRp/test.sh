#!/bin/sh
set -eu
cd "$(dirname "$0")"
for compiler in gcc clang; do
    command -v "$compiler" >/dev/null 2>&1 || continue
    binary="$(mktemp)"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        filter_graphics_fragments.c filter_graphics_fragments_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        row_control.c row_control_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        rle_windows_2018.c rle_windows_2018_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        filter_graphics_fragments.c rle_windows_2018.c \
        rle_pipeline_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        label_setup.c label_setup_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        label_setup.c row_control.c label_setup_row_test.c -o "$binary"
    "$binary"
    rm -f "$binary"
done
