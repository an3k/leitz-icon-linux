#!/bin/sh
set -eu
cd "$(dirname "$0")"
for compiler in gcc clang; do
    command -v "$compiler" >/dev/null 2>&1 || continue
    binary="$(mktemp)"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        devmode_patch.c devmode_patch_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        paper_geometry.c paper_geometry_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        printer_data.c printer_data_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        status_probe.c status_probe_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        sendrecv_result.c sendrecv_result_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        wireless_payload.c wireless_payload_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        sendrecv_sequence.c sendrecv_result.c sendrecv_sequence_test.c -o "$binary"
    "$binary"
    rm -f "$binary"
done
