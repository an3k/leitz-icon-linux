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
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        raster_stream.c raster_stream_test.c row_control.c label_setup.c \
        filter_graphics_fragments.c rle_windows_2018.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_host.c oem_host_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_factory.c oem_host.c oem_factory_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_method_map.c oem_method_map_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_lifetime.c oem_lifetime_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_row_dispatch.c oem_row_dispatch_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_row_dispatch.c oem_row_dispatch_integration_test.c \
        raster_stream.c row_control.c label_setup.c \
        filter_graphics_fragments.c rle_windows_2018.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_continuous.c oem_continuous_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_continuous.c oem_continuous_bridge.c oem_row_dispatch.c \
        raster_stream.c row_control.c label_setup.c \
        filter_graphics_fragments.c rle_windows_2018.c \
        oem_continuous_integration_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_write_data.c oem_write_data_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_write_data.c oem_slot8_finish.c oem_slot8_finish_test.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_write_data.c oem_write_integration_test.c \
        raster_stream.c row_control.c label_setup.c \
        filter_graphics_fragments.c rle_windows_2018.c -o "$binary"
    "$binary"
    "$compiler" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
        oem_gdi_caps.c oem_gdi_caps_test.c label_setup.c -o "$binary"
    "$binary"
    rm -f "$binary"
done
