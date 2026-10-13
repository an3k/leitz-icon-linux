# Classic macOS 5.1.465 — recovered C for the command filter's active path

Reference: the vendor's `command2leitz` Mach-O x86_64 executable (35,932 bytes; SHA-256 `6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee`).

This folder holds independent source reconstruction, **not** manufacturer-supplied C. The installer has no `command2leitz.c` or `raster2printer.c`.

## Reconstructed from original assembly

- `_main` at `0x100000b10..0x1000011c0`: open command file, PPD option parsing, three case-insensitive command names, command loop. In particular, `ReportStatus` returns exit code **1 immediately** after its query; `GetUnitedStatus` and `GetFirmwareVersion` continue.
- `_GetPrinterStatus`, `_GetUnitedStatus`, `_GetFirmvareVersion`: query bytes `ESC A/U/V` and 7-second read deadline.
- `_GetStatus` at `0x100001440..0x100001a26`: backchannel read, response state matching, unescaped `%s%s` forwarding for U/V, original `/private/var/log/leitz.log` location and `com.xyron.Leitz-Icon-Software` notification name. An absent response does not emit `ESC Z`.
- `_EndOfJob` at `0x100001a30..0x100001a76`: emits `ESC Z`, flushes stdout, and ignores its argument `ShouldSend_ESC_A`.
- Original text prefixes recovered from Mach-O's `__DATA` pointers: `PRINTER_STATUS_`, `UNITED_STATUS_`, `FIRMWARE_VERSION_`.

The CUPS entry adapter compiles against Linux CUPS 2.x for **offline** inspection and uses platform CUPS headers on macOS; it is **not an installed printer filter**. Its real macOS compilation remains unverified because macOS SDK headers and runtime are unavailable in this environment.

## Checks completed

- GCC with `-std=c11 -Wall -Wextra -Werror`: built and exercised the recovered command path.
- Clang AddressSanitizer and UndefinedBehaviorSanitizer: isolated branch checks passed.
- Linked the CUPS adapter against installed `libcups.so.2`, and invoked it with an empty command file and the vendor PPD: exit 0.
- **No original Mach-O executable was executed and no device output was compared.** These checks do not establish 1:1 functional equivalence.

## Limitations: a finished 1:1 reconstruction is NOT available

1. The original command executable also links `_OpenDebugFile`, `_OpenLogFile`, `_WriteData`, `_SendDebugFileToPrinter`, `_output_cups_info`, `_test_raster` and other utilities. These are not reproduced in this isolated active-path source.
2. Diagnostic stdout/stderr formatting and auxiliary side effects are not fully re-created.
3. The vendor writes a NUL at `reply[received]` into its nominal 1024-byte stack buffer. The recovery uses 1025 bytes for the 1024-byte reply boundary: this is an explicitly identified memory-layout deviation, not a verified match for undefined original behavior.
4. The original Mach-O cannot be executed on this Linux host for direct parity comparisons; the real macOS build and device communication are unverified.
5. The **other** classic executable, `raster2printer`, is **not complete**. The prior experimental raster components remain separately under `enhancements/macos-5.1.465-classic/reconstructed/`.

Do not confuse this partial recovery with the two complete, original manufacturer executables. Do not install it as a production printer filter.
