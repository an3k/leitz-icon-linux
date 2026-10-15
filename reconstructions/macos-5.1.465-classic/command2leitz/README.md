# Classic macOS 5.1.465 — recovered C for the command filter's active path

Reference: the vendor's `command2leitz` Mach-O x86_64 executable (35,932 bytes; SHA-256 `6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee`).

This folder holds independent source reconstruction, **not** manufacturer-supplied C. The installer has no `command2leitz.c` or `raster2printer.c`.

## Reconstructed from original assembly

- `_main` at `0x100000b10..0x1000011c0`: open command file, PPD option parsing, three case-insensitive command names, command loop. In particular, `ReportStatus` returns exit code **1 immediately** after its query; `GetUnitedStatus` and `GetFirmwareVersion` continue.
- `_GetPrinterStatus`, `_GetUnitedStatus`, `_GetFirmvareVersion`: query bytes `ESC A/U/V` and 7-second read deadline.
- `_GetStatus` at `0x100001440..0x100001a26`: backchannel read, response state matching, unescaped `%s%s` forwarding for U/V, original `/private/var/log/leitz.log` location and `com.xyron.Leitz-Icon-Software` notification name. An absent response does not emit `ESC Z`.
- `_EndOfJob` at `0x100001a30..0x100001a76`: emits `ESC Z`, flushes stdout, and ignores its argument `ShouldSend_ESC_A`.
- Original text prefixes recovered from Mach-O's `__DATA` pointers: `PRINTER_STATUS_`, `UNITED_STATUS_`, `FIRMWARE_VERSION_`.

The CUPS entry adapters compile against Linux CUPS 2.x for **offline** inspection, and both the legacy and connected-original `command2leitz` entry candidates now compile against the native macOS SDK in GitHub Actions. They remain **non-production comparison filters**, not installed printer drivers.

## Checks completed

- GCC with `-std=c11 -Wall -Wextra -Werror`: built and exercised the recovered command path.
- Clang AddressSanitizer and UndefinedBehaviorSanitizer: isolated branch checks passed.
- Linked the CUPS adapter against installed `libcups.so.2`, and invoked it with an empty command file and the vendor PPD: exit 0.
- Subsequently the SHA-256-verified original Mach-O **was executed on native macOS** alongside independently built source-derived entry candidates. Exact stdin/stdout/stderr/exit parity now exists for documented synthetic branches, but not for unrestricted device behavior or the native ABI.

## Limitations: a finished 1:1 reconstruction is NOT available

1. The isolated active command path is not yet linked with the separately recovered file/debug helpers, original `_WriteData` error diagnostics, or the shared OEM CUPS/raster diagnostic functions. Those are stored as independent C units and do not constitute a whole-executable reconstruction.
2. Diagnostic stdout/stderr formatting and auxiliary side effects are not fully re-created.
3. The vendor writes a NUL at `reply[received]` into a 1024-byte stack buffer. The recovered code retains that operation without a new bounds check; for a 1024-byte answer both implementations can write one byte out of bounds. This is a known original defect, not safe or production-qualified behavior.
4. The original Mach-O has been executed on a native macOS CI runner for deterministic comparisons; live printer communication and remaining macOS file/network/global edge cases are unverified.
5. The **other** classic executable, `raster2printer`, is **not complete**. The prior experimental raster components remain separately under `enhancements/macos-5.1.465-classic/`.

Do not confuse this partial recovery with the two complete, original manufacturer executables. Do not install it as a production printer filter.

## 2026-10-14: recovered original command-file helper bodies

Original manufacturer `command2leitz` Mach-O has been disassembled and its four previously missing file-handling functions reconstructed in `original_command_file_helpers.[ch]`:

- `_OpenDebugFile` (0x100001c90–0x100001f30): original fixed `/Library/Caches/DEBUG/dbg.prn` path, conditional `mkdir/chmod`, unguarded `sprintf`, `fopen("wb")` and original line-numbered messages (88, 104, 108).
- `_OpenLogFile` (0x100001f40–0x100002167): fixed `/Library/Caches/DEBUG/LeitzDriverLog.log`, original file permissions and unchanged line numbers (116, 132).
- `_SendDebugFileToPrinter` (0x100002170–0x10000243c): one-byte `fgetc` / `fputc` loop, original error termination, unmodified diagnostics (142, 152, 158, 162) and unconditional zero return.
- `_WriteDebugBuffer` (0x1000024c0–0x100002518): original three nonzero checks and single `fwrite(data,length,1,stream)` with ignored result.

**No enhancements were introduced into these reconstructed functions.** The 1024-byte globals and original unsafe fixed-path opening are retained. The standalone tests exercise only already-open streams and a temporary PRN file; the fixed `/Library/Caches` filesystem writes are deliberately not invoked by tests. Original code and its boundary conditions must not be used as production-safe printing routines. Local GCC, Clang and Clang ASan/UBSan checks passed; CI build results are to be verified against the actual run. No change was made to the existing command entrypoint, the unrelated raster filter, originals or `enhancements/`.

## 2026-10-14 — original command2leitz _main control flow and CUPS candidate

Examined the verified original `command2leitz` x86-64 Mach-O `_main` at
`0x100000b10..0x1000011c0` (SHA-256
`6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee`).
The previous partial CUPS adapter remains unchanged.

New `original_command_main_flow.[ch]` recovers the defined OEM
`_main` branches: unconditional start message; `argc == 7` command-file
open (return 1 on failure) vs stdin otherwise; `ppdOpenFile` with the
original `perror` and -1 return on failure; `cupsParseOptions`,
`ppdMarkDefaults`, `cupsMarkOptions`, marked choice comparison
for `ShouldSend_ESC_A`, option freeing and PPD close; repeated
`cupsFileGetConf` with 1024-byte option name; `ReportStatus` exits
1 regardless of query result; other two commands continue; unknown
commands are ignored. Original stderr diagnostic line numbers and
strings 41/49/54/75/85/98 are reproduced, including their line breaks.
No new argc validation, retry limit or cleanup is added.

The interface passes CUPS and status calls through callbacks. This
is an original-function **behavioral C reconstruction**, NOT the
manufacturer's native ABI or a fully reconstructed binary. The
existing binary `command2leitz_recovered_cups` is untouched.
New `original_command_main_cups_candidate.c` connects the branch-
faithful function to real libcups, using the existing reconstructed
status-query code. The candidate is additional and non-production.

Local `gcc`, `clang`, and Clang ASan/UBSan tests passed. They
verify exact stderr bytes on the defined normal path, all CUPS
callback orders, choice YES/NO/absent, missing file/PPD, and early
exit for `ReportStatus`. The real libcups candidate was compiled
and launched offline with the original PPD and an unknown command;
this produced the expected original `_main` diagnostics.

New `verification/macos-5.1.465-classic/compare_command_main_with_vendor.sh`
uses the macOS runner and independently verified manufacturer
Mach-O to compare **complete stdout, stderr and exit status**
for valid-file, stdin, nonexistent-file and nonexistent-PPD entry
cases. This test does not access a real printer or change existing
OEM comparisons. Its result must be verified after the commit.

Remaining: native error/undefined branches outside these defined
tests, direct comparison of genuine status calls, original global
and C++ ABI and full original executable behavior. No enhancement
is introduced into manufacturer-derived routines.

## 2026-10-14 — verified OEM _WriteData zero-only success criterion

The manufacturer x86_64 Mach-O `_WriteData` routines were checked again at `command2leitz 0x100001a80..0x100001c14` and `raster2printer 0x100001450..0x1000015e4`. Both executables compare the primary `send` or `fwrite` result **only against zero** and return 1 for any nonzero value, including a signed `send` result of -1. Both OEM versions emit the exact error text `\nraster_c_functions.c:55:WriteData(): an error occured while writing data of size : %ld` **only** when the selected sink returns zero. On non-socket output, the original returns 0 immediately for zero length or no stdout stream, without an error message. The debug dump's return value is ignored.

The two preexisting, incomplete sink-choice C implementations were corrected **in place only on those differences**; no alternative implementations, retry loops, additional guards, or changes to the established good paths were introduced. The separate cross-executable test in `verification/macos-5.1.465-classic/original_write_data_semantics_test.c` checks signed -1, positive short sends, zero sends, zero/positive stdout writes, PRN-only output, zero-length writes, dump ordering, and complete OEM stderr bytes. Local GCC and Clang with AddressSanitizer/UndefinedBehaviorSanitizer passed. CI builds now run this cross-test on Linux GCC/Clang and macOS Clang.

The test is based on the **verified original assembly**, not on executing the original internal `_WriteData` directly. This still does not establish exact original ABI, network/device behavior or full program equivalence.

## 2026-10-14 — complete defined status diagnostics in joined OEM entry

The manufacturer `_GetPrinterStatus`, `_GetUnitedStatus`,
`_GetFirmvareVersion` and `_GetStatus` text, error, log-file
and state-message diagnostics were recovered on the original-derived
command path without touching original vendor binaries or changing
the command bytes. Native macOS proof:
[run 38564165909](https://github.com/an3k/leitz-icon-linux/actions/runs/38564165909).
All 13 independent original-versus-connected executable comparisons
matched **exit code, full stdout, full stderr** exactly, with
synthetic backchannel interception confirmed. Cases include normal,
paper errors, unknown status, no reply, read error and U/V string
replies. Existing original raster output parity remained intact.

The compiled, reconstructed command entry is an analysis-only
source-to-CUPS connector, not the original C++ ABI; the broader
native error paths and genuine device interaction remain incomplete.

## 2026-10-14 — native global OEM utility integration

`original_command_file_helpers.[ch]` now exposes reconstructed
`FlushDataStreams`, `is_bit_set`, `CloseDebugFile` and
`CloseLogFile` native C function surfaces. Their bodies delegate
to the existing original-derived utility implementation, but use
the actual OEM global file pointers `__fd_dump`/`__fd_log`.
Both the normal data-flush and absent-debug-dump paths, file
closing and 32-bit original shift wraparound were checked.

The original-derived command CUPS candidate links this module
and calls `FlushDataStreams` for output flushes. No separate
new implementation of the original algorithms was introduced.
[All 3 CI jobs passed](https://github.com/an3k/leitz-icon-linux/actions/runs/38572871936),
including unchanged complete stdout/stderr/exit matching for
13 distinct manufacturer command cases. The new close/bit
functions are linked and unit-tested but not necessarily
executed by those 13 OEM command scenarios. Full native
binary ABI and original hardware status behavior remain open.

## 2026-10-14 — original-named command entry points and previously isolated functions

`GetStatus`, `GetPrinterStatus`, `GetUnitedStatus`, `GetFirmvareVersion`, `EndOfJob`, and `WriteData` are now present as named C function boundaries in the joined original-derived command CUPS executable. The existing original query body and original `_WriteData` sink-selection code are reused, and command main now selects the original named per-command query entry. None of this changes the actual command bytes, statuses, diagnostics or original unchecked error semantics. `WriteData` remains dormant in normal status processing.

The three earlier reconstructed dormant OEM helpers in `../shared/diagnostics/`—`output_cups_info`, `test_raster`, and `PrintPageDebugInfo`—are linked from their existing single shared C sources. Normal status processing does not call them. [Native full validation run 38604431098](https://github.com/an3k/leitz-icon-linux/actions/runs/38604431098) compiled every function against native macOS CUPS, checked actual linked symbols, ran unit tests, and preserved all 13 exact original-command stdout/stderr/exit comparisons. The wrapper callback signatures are **not** asserted to match the OEM's binary ABI, and this candidate is not an installable driver.

## 2026-10-15 — command `_WriteData` native entry and stream globals

The joined OEM-derived macOS `command2leitz` now has `original_command_native_write_data.[ch]` with manufacturer-oriented arguments **(data, length, socket fd, by-value 0x10a8 descriptor working image)**, replacing the earlier callback-struct parameter at the `WriteData` function symbol. The original recovered `classic_command_write_data` core remains the one and only sink/return-decision implementation. Native globals `__fd_stdout`, `__fd_dump` and `__fd_log` use exact Mach-O assembler symbol names.

`original_command_native_write_data_test` verifies temporary FILE* outputs, dump-only output, AF_UNIX socket send and the manufacturer's intentional negative-send success semantics with no external device; it is automatically run by GCC, Clang and native macOS. [Actions 38650073372](https://github.com/an3k/leitz-icon-linux/actions/runs/38650073372) passed every CI job and all established manufacturer comparisons. **Important:** the original command Mach-O does not invoke `_WriteData` in these normal status cases. The native C++ compiler-generated by-value ABI and any still-unknown descriptor fields are *not* certified; no production driver was altered.

## 2026-10-15 — native OEM status entry signatures

The original macOS command binary passes `(bool, double)` to `_GetPrinterStatus`, `_GetUnitedStatus` and the originally spelled `_GetFirmvareVersion`, and `(char, bool, const char *, double)` to `_GetStatus`. The original `_EndOfJob` takes one bool. Exact argument registers are recovered in the 2026-10-15 symbol inventory and in [OEM disassembly audit 38665347152](https://github.com/an3k/leitz-icon-linux/actions/runs/38665347152). All five native entry points are exported by the joined macOS CUPS candidate through `original_command_native_status_entry.h`. They reuse the original-derived status/ESC Z callback bodies; the ordinary command main routes through the three native query symbols and common native status symbol. Native EndOfJob remains a separate entry, with the internal callback still used by the shared behavioral core.

[CI 38665905068](https://github.com/an3k/leitz-icon-linux/actions/runs/38665905068) passed all three jobs with native symbols/compile checks and **25/25 complete original command stdout/stderr/exit matches**; previous raster/PRN comparisons remain successful. The C++ private global layout, unexplored original error states and live printer operation are not certified.
