# Original macOS classic driver — installer 5.1.465, PPD 1.2.190

| Component | Original size | SHA-256 | Branch contents |
|---|---:|---|---|
| `LeitzIcon.ppd` | 151,524 bytes | See git blob `14628cebde52eba5cc731b4679eda1471f50e312` | Original PPD, byte-identical |
| `raster2printer` | 84,252 bytes | `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13` | Symbol inventory; full disassembly held in separate analysis bundle |
| `command2leitz` | 35,932 bytes | `6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee` | Symbol inventory; full disassembly held in separate analysis bundle |

The two original executable files are Mach-O x86-64. Original debug metadata identifies `raster2printer.c`, `command2leitz.c`, and the shared `raster_c_functions.c`; these filenames **do not establish that the full original C source is present**.

Completed: binary identification, disassembly, initial function/branch identification, acquisition of PPD, symbol inventories.

**Not completed:** full decompilation into C, correct recreation of globals/constants and macOS dependencies, successful compilation, functional comparison on controlled equivalent inputs, hardware qualification.

Linux original C is useful as a reference for APIs and terminology only; no identical implementation is assumed. V3 is a distinct model-specific driver and is intentionally excluded from this stage.

No fabricated C code has been created merely to satisfy a file count.

## Disassembly-derived, compilable C fragments

The `reconstructed/` directory now contains real C source files for small portions of both classic macOS binaries. Built locally using GCC C11 with `-Wall -Wextra -Werror`; `make check` succeeded. No macOS runtime or USB hardware test was performed.

- `mac_classic_command_core.[ch]`: query letters for GetPrinterStatus / GetUnitedStatus / GetFirmvareVersion; ESC A response equality checks for 0x03, 0x20, 0x30, 0x40; ESC Z end marker.
- `mac_classic_raster_boundaries.[ch]`: StartDocument 120 padding ESC bytes, ESC @, optional ESC i; EndPage ESC G plus optional ESC E; EndDocument optional ESC E plus ESC @ ESC Z.
- `smoke.c` and `Makefile`: compile these isolated code fragments and check reconstructed byte sequences without CUPS or USB access.

Relevant disassembly addresses (Mach-O x86-64 virtual addresses):

| Original executable | Function | Address |
|---|---|---|
| command2leitz | GetPrinterStatus | 0x1000011d0 |
| command2leitz | GetUnitedStatus | 0x1000012a0 |
| command2leitz | GetFirmvareVersion | 0x100001370 |
| command2leitz | GetStatus | 0x100001440 (only fragments covered) |
| command2leitz | EndOfJob | 0x100001a30 |
| raster2printer | StartDocument | 0x100006890 |
| raster2printer | EndPage | 0x1000089b0 |
| raster2printer | EndDocument | 0x100008ac0 |

**Not reconstructed:** both complete `main` functions, CUPS option parsing, raster input, complete half-toning, page geometry, StartPage, ProcessPage, ReadPage, USB/socket output, macOS notifications, complete status backchannel, and all shared functions. The fragments are **not** a replacement for the originals. A successful fragment build is not a successful complete driver build.

## Reconstruction update: raster geometry and original half-toning

Reconstructed source in `reconstructed/` now additionally contains:

- `mac_classic_dither.c` / `.h`: independent disassembly-based reconstruction of `_DitherPage` (`0x100006dd0`). All **13 distinct manufacturer integer matrices** have been extracted directly from the classic Mach-O data segment, with source addresses and SHA-256 of each matrix block preserved in comments. The switch table maps 14 modes to these tables, dimensions, and floating-point scaling factors. Mode 7 falls through without an initialized table pointer in the observed binary and is rejected by the reconstructed safe API rather than dereferenced.
- `mac_classic_raster_boundaries.c` / `.h`: `_StartPage` (`0x100007380`) now emits the **original 11-byte page setup** `ESC L <length-hi> <length-lo> ESC w <width-hi> <width-lo> ESC B 0`. Its 300/600 dpi-dependent length arithmetic comes directly from the binary.
- `smoke.c` tests those page-setup bytes, dither mode dimensions/scales, representative black/white raster output and malformed input rejection.

For reproducibility on Linux with GCC:

```sh
cd sources/macos-5.1.465-classic/reconstructed
make clean check all
```

This successfully compiled and ran locally under `-std=c11 -Wall -Wextra -Werror`.

**Limitations:** This is still a **partial C reconstruction**. The full raster loop (`_ProcessPage`), document option handling, original CUPS API integration, complete `command2leitz` response/error control flow and executable entry points are **not** fully reconstructed. Behavioral equivalence to a running Mach-O filter has not been demonstrated. Matrix bytes are verified independently of behavior.

### Original `PrintQuality` selection

The `_raster2printer` case-insensitive choice comparisons at Mach-O virtual addresses `0x100004dbd..0x100005098` have now been reconstructed in `mac_classic_quality_from_ppd()`. This yields all **15 original PPD choices**, mapped to the exact integer selectors (0..14) used by `_DitherPage`. Choice `Draft` maps to selector 7 and sets single-bit raster input in the original PPD; it is intentionally not processed by the RGB24 dither routine. The 14 other selections map to 13 extracted manufacturer matrix arrays.

The selection mapping and its case-insensitive behavior are included in the successful local `make check` run. This documents and reconstructs **the quality selector**, not the remainder of the original printing application.

## ProcessPage raster encoder recovery (current milestone)

From the classic original `raster2printer`, `_ProcessPage` at `0x100007560..0x1000089a6`:

| `OptimizationLevel` | Mach-O code branch | Observed treatment |
|---|---|---|
| 0 | `0x10000889b` | Raw full-width rows including white rows; row-submission fragment reconstructed |
| 1 | `0x100007e6e` | White-row skip (`ESC f`), full-width raw row; row-submission fragment reconstructed |
| 2 | `0x100007f32` | White-row skip, byte-edge trimming, ESC B/D, bit-run compression (0x17), fallback to 0x16 raw |
| 3 | `0x100007a74` | White-row skip, byte-edge trimming, ESC B/D, 0x16 raw |

The first milestone of `reconstructed/mac_classic_process_page.[ch]` implemented **isolated mode 2 and 3 row-framing**, including `ESC f` white-row flushing when the next printed row is encountered, `ESC B` left offset, `ESC D` width state, 0x16 raw data and 0x17 bit-run compression when shorter. The compressor reads high-to-low bits and emits run bytes (black bit in the high bit, count-minus-one in the lower seven bits). It preserves a non-obvious asymmetry in the original machine code: for mode 3, trailing white bytes are **not** trimmed when the first byte of a row is already nonzero (`0x100007ab4`); mode 2 always performs trailing-byte trimming (`0x1000080b6`).

The code uses explicit bounded output buffers rather than the unsafe pointer arithmetic present in the original Mach-O binary. A failed output capacity request therefore returns an error. This is a **safe C reconstruction of identified fragments**, not a byte-by-byte decompilation of all original `ProcessPage`.

The local `make clean check all` build and the same smoke assertions under AddressSanitizer/UndefinedBehaviorSanitizer completed successfully on Linux. They check selected expected command bytes and edge cases, not behavioral identity with the original macOS executable.

**Remaining before full, re-compilable classic macOS source:** original page buffer layout, page-level option/length/offset handling, complete CUPS entry and output flow, full status/USB communication branches, and comparison against the manufacturer's executable. The assembled fragments must not be used as a functional replacement driver.

## ProcessPage modes 0/1 and ReadPage recovery

Based on independent Mach-O disassembly from the **classic** `raster2printer` executable:

- `_ProcessPage` at `0x100007633..0x100007739`: modes 0 and 1 emit one initial four-byte `ESC D <width> SYN` header.
- `_ProcessPage` at `0x10000889b..0x100008945`: mode 0 transfers every full packed bitmap row without suppressing empty rows.
- `_ProcessPage` at `0x100007939..0x100007a74` and `0x100007e6e..0x100007f32`: mode 1 buffers empty rows into `ESC f <count>` and transfers nonempty full-width rows as-is.
- `_ReadPage` at `0x100008d20..0x10000918b`: reads a CUPS header, increments page counter, allocates for `height * cupsBytesPerLine`, calls `cupsRasterReadPixels` once, and treats zero returned bytes as a read failure.

`reconstructed/mac_classic_process_page.[ch]` now exposes a row emitter for **all four optimization levels 0–3**; `mac_classic_row_preamble()` emits the one-time header for 0/1. These reconstructed units are **only the isolated row-submission paths**, not the full original `_ProcessPage` function.

`reconstructed/mac_classic_read_page.[ch]` separately recovers the core `_ReadPage` allocation and pixel-read sequence behind a callback interface, allowing independent compilation without macOS/CUPS headers. **Deliberate hardening:** unlike the original's zero/nonzero read-result check, it rejects any short read. The page counter is incremented only after a complete page is read, not immediately after header acceptance. These deviations are documented rather than presented as exact manufacturer behavior.

The combined modules build with GCC C11 `-Wall -Wextra -Werror`; local fragment checks passed both a normal build and ASan/UBSan. There is **still no complete executable macOS filter**, no CUPS-native adapter for this callback layer, and no original-binary execution comparison.

## Connected portable job coordinator and CUPS adapter (13 October 2026)

An additional **independent integration layer** now connects the recovered classic macOS functions in their observed manufacturer call order, based on `_raster2printer` disassembly between `0x1000045a0` and `0x100006854`.

- `mac_classic_job.c/.h`: document initialization, page read, 24-bit RGB dithering or 1-bit pass-through, page setup, four selectable raster row encoder modes, per-page cutter command and document shutdown. Writer and raster callbacks allow isolated compilation with no printer access. Completed pages are counted only after their output succeeds; failed writes and short page reads abort.
- `mac_classic_cups_filter.c`: **optional CUPS 2.x adapter** implementing the original filter's 6/7-argument calling convention, original PPD lookup and marking calls, supported option extraction, CUPS raster page reading, and stdout binary output. Undeveloped options such as vendor PRN/debug/network paths are rejected, not faked.
- `job_smoke.c`: isolated checks for two consecutive pages, RGB24-to-1-bit, Draft selection, original mode-1 skip framing, short input, incomplete output and empty jobs.
- `Makefile`: `make check all` compiles the portable units and runs both smoke checks. `make mac_classic_cups_filter` is separate and requires real CUPS 2.x development files.

**Verified locally:** GCC `-std=c11 -Wall -Wextra -Werror` regular compilation and both smoke programs; `job_smoke` also passed ASan/UBSan. The CUPS adapter was syntactically checked using locally defined CUPS-type/function declarations because actual CUPS headers and libraries are not installed in this environment. **The real CUPS adapter has not been compiled or run against CUPS itself.**

**Not yet established:** faithful full macOS CUPS filter execution, original debug/PRN/socket pathways, options for media-specific transformations, several `ProcessPage` branches, full macOS status decoder, correctness of printer hardware operation, or equivalence against executions of the original Mach-O binaries. These files must **not** be deployed to a physical printer. They are readable, compilable C source with documented reconstructed and newly authored portions, not recovered original full C source.

## Recovered classic WriteData / status exchange (13 October 2026)

The classic executables have parallel `_WriteData` routines (`raster2printer` address `0x100001450`, `command2leitz` address `0x100001a80`). Their disassembly confirms:
- a conditional socket `send` path (selection includes a `strstr(...,"ipp")` check),
- a `__fd_stdout` `fwrite` path with optional preceding `__fd_dump` `fwrite`,
- the original's **positive-result success check** accepts short socket sends as successful and does not validate full byte counts. This is a manufacturer implementation defect, not a feature.

`reconstructed/mac_classic_output.[ch]` safely models these as **explicitly selected** socket or stdout sinks, with optional PRN dump on the stdout route. It loops until all bytes are transmitted, rejects zero/negative responses, and propagates dump errors. Such guards differ intentionally from the original; it does not reproduce the unsafe implicit selection based on caller memory. `mac_classic_cups_filter.c` now passes its stdout bytes through this common output helper. This CUPS adapter itself is not yet built against real CUPS headers.

Classic `command2leitz` entry (`_main`, `0x1000010b4..0x100001176`) uses **case-insensitive** CUPS command names `ReportStatus`, `GetUnitedStatus`, `GetFirmwareVersion`, mapping to `ESC A`, `ESC U`, `ESC V`, respectively. `GetPrinterStatus` is the **internal C function name**, not the external command name. The PPD choice `ShouldSend_ESC_A` is compared with `YES`; its resulting flag flows into `_EndOfJob` but the observed implementation of that function emits `ESC Z` unconditionally (`0x100001a30`).

`reconstructed/mac_classic_command_dispatch.[ch]` captures command selection and option interpretation.

Classic `_GetStatus` (`0x100001440..0x100001a26`) writes the two-byte command, calls `cupsBackChannelRead` into a 1024-byte buffer, and distinguishes -1 (read failure), 0 (no data), and >0 (response). For **non-empty** responses it calls `_EndOfJob` to emit `ESC Z`; read failures and empty responses do **not** emit `ESC Z`. For `ESC A`, the first byte is compared with 0x03, 0x20, 0x30 and 0x40, each producing original CUPS `STATE` reasons; before the comparison it prints a state-clearing line. For `ESC U` and `ESC V`, the original follows a separate `/private/var/log/leitz.log` / notification / `marker-message` path, not yet recovered fully.

`reconstructed/mac_classic_status_exchange.[ch]` implements isolated command/backchannel exchange, keeps the complete raw response, maps the observed `ESC A` status byte **without bitmask assumptions**, and provides original state reason strings. Unlike the original, it does not append a terminating NUL **outside** a full 1024-byte response buffer.

`reconstructed/io_smoke.c` checks short output writes, dump mirroring, I/O failures, command names, empty/error/normal backchannel responses, expected `ESC Z`, and the original status reason text. The standalone fragments compile using GCC and Clang with `-std=c11 -Wall -Wextra -Werror`; local `make check all` passed and the sanitizer build `-fsanitize=address,undefined` passed.

**Scope remains partial:** There is not yet a fully reconstructed manufacturer CUPS command executable, no exact reconstruction of the U/V notification formatting, no full macOS PRN/socket setup, and no byte-for-byte comparison with running Mach-O originals. No hardware confirmation. Reconstructed C includes explicit safety improvements and is not claimed to be the manufacturer's exact source.

## Classic command2leitz executable reconstruction (2026-10-13)

**Functional, compilable command entrypoint now added:** `reconstructed/mac_classic_command_filter.c`, `mac_classic_command_job.[ch]`, `mac_classic_command_abi.h`, `command_job_smoke.c`, and updated Makefile.

Evidence from original classic macOS `command2leitz` Mach-O:
- `_main` (`0x100000b10..0x1000011c0`) reads CUPS command files (`cupsFileOpen` / `cupsFileStdin` + `cupsFileGetConf`) and reads printer options. The original reads the `ShouldSend_ESC_A` PPD choice, but its `_EndOfJob` issues `ESC Z` unconditionally.
- Original `__TEXT,__const` at `0x100004d60` stores **7.0 seconds** as the timeout supplied to `cupsBackChannelRead`.
- `_GetStatus` (`0x100001440..`) distinguishes read failure, empty response, and received data. Only the received-data path emits `ESC Z`. Exact ESC A comparison values: 0x03/0x20/0x30/0x40.
- Non-A replies are combined with the command value for `ATTR: marker-message`, written to `/private/var/log/leitz.log`, and notified with domain **`com.xyron.Leitz-Icon-Software`**.
- **Existing OEM mismatch:** original PPD's `*cupsCommands` advertises `GetPrinterStatus GetUnitedStatus GetFirmwareVersion ReportState`; the original `_main` *handles* `ReportStatus GetUnitedStatus GetFirmwareVersion`. `GetPrinterStatus` is an internal function name. Our reconstruction preserves the original handled names and does not invent aliases.

### Actual build result

`make clean all check` built the command executable **against real Debian CUPS 2.x runtime `libcups.so.2`** and passed all four portable smoke programs with GCC `-std=c11 -Wall -Wextra -Werror`. The command entrypoint used only opaque public CUPS function signatures because the local system lacks the development headers; on macOS the Makefile selects the CUPS SDK headers and `-lcups`. This is **not** yet a macOS build.

A **real CUPS command-file parser + shared-library exercise**, with a locally interposed *fake backchannel* and **no printer attached**, showed:
- Input `ReportStatus`, reply `0x03`: output bytes `1B 41 1B 5A`, correct CUPS `STATE` remove/ready messages, exit status 0.
- Input `GetUnitedStatus`, reply `0x03`: output bytes `1B 55 1B 5A`, safe marker-message encoding, and opt-in local archive file.
- Input unsupported `ReportState`: nonzero exit, no synthetic device query.
- `command_job_smoke` passed under Clang AddressSanitizer and UndefinedBehaviorSanitizer.

### Intentional differences from vendor

Output writes retry after EINTR; partial writes are completed or rejected, rather than regarded as success. Binary and control characters in CUPS marker values are encoded as printable escapes. Original `/private/var/log/leitz.log` replacement and `notify_post` side effects are **disabled by default**; set `LEITZ_MAC_CLASSIC_STATUS_LOG` to explicitly request a log copy, and `notify_post` is compiled only on macOS. Actual macOS notification behavior has **not** been exercised.

The CUPS command adapter does not yet reproduce original PPD-marking internals, vendor debug/network paths, or all macOS UI integration. This is a source-identified **new C reconstruction**, not verbatim lost OEM C source, and is **not hardware-qualified**.

### Remaining overall classic driver work

`raster2printer` still lacks complete macOS-specific page/media option handling, original PRN/socket setup, native CUPS SDK compilation, and byte-level equivalence testing with an executing Mach-O original. `command2leitz` still requires verification of U/V notification formatting and final platform-specific behavior. Do not distribute these experiments as production drivers.

## Deferred PrintThroughPRN output path — 13 October 2026

The original classic macOS `raster2printer` executable implements a **separate deferred PRN transfer path**:

1. `_OpenDebugFile` at `0x100001660` creates `/Library/Caches/DEBUG/dbg.prn` using `fopen("wb")`, plus `mkdir`/`chmod` calls, including potential overwriting of an existing PRN.
2. `_WriteData` at `0x1000014cb..0x10000150d` writes bytes to a debug file if present, and returns early if the normal stdout stream is unset.
3. The `PrintThroughPRN` option sets stdout output to NULL at `0x10000541d..0x100005460`.
4. At the end, `_CloseDebugFile` precedes `_SendDebugFileToPrinter` (`0x100006350..0x100006367`). `_SendDebugFileToPrinter` (`0x100001b40`) reopens `dbg.prn` and copies it to stdout one byte at a time using `fgetc`/`fputc`.

Added `reconstructed/mac_classic_prn.c/.h` to implement **deferred PRN buffering and replay** without creating the manufacturer's global, destructive directory or overwriting `dbg.prn`. Unlike the original, `tmpfile()` stores the encoded bytes anonymously; on successful job encoding, replay copies blocks through the existing checked writer to stdout. A failed encoding operation never replays an incomplete document. This is an intentional safer deviation, *not* byte-identical restored original C.

`mac_classic_cups_filter.c` now supports `PrintThroughPRN=ON` via this module; `PrintThroughPRN=OFF` uses the existing streaming stdout writer. The default and other unsupported options are unchanged. `Makefile` includes the new module, and existing `io_smoke.c` validates that output does not appear before replay.

**Local verification:** `make clean all check` succeeded with GCC C11 `-Wall -Wextra -Werror`, CUPS adapter syntax checked against local *mock headers* (not the SDK), and PRN writer smoke passed with ASan/UBSan. No printer, macOS/CUPS SDK build, or original Mach-O runtime equivalence confirmed.

**Still missing for full classic driver reconstruction:** exact PRN/debug file behavior, `ResolveDeviceURI` and socket creation, page-reprint/resume path, platform-specific status side effects, complete option/layout semantics, and manufacturer-runtime output comparisons.
