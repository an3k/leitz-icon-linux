#!/bin/sh
set -eu
cd "$(dirname "$0")"
for compiler in gcc clang; do
    command -v "$compiler" >/dev/null 2>&1 || continue
    binary="$(mktemp)"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        status_dispatch.c status_dispatch_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        monitor2_native_map.c monitor2_native_map_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        monitor2_port_io.c monitor2_port_io_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        monitor2_native_map.c monitor2_port_io.c status_dispatch.c \
        monitor2_status_integration_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        monitor2_document_lifecycle.c monitor2_document_lifecycle_test.c \
        -o "$binary"
    "$binary"
    rm -f "$binary"
done
