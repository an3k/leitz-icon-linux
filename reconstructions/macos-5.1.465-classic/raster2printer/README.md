# Classic macOS 5.1.465: independently recovered raster job boundaries

Reference binary: `raster2printer`, SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`.

The C implementation corresponds to specific disassembled functions, not the entire manufacturer executable:

- `_StartDocument` (`0x100006890`): 120 leading ESC padding bytes followed by ESC @, optionally ESC i for 600 dpi.
- `_StartPage` (`0x100007380`): ESC L with job-derived height, ESC w with raster width, ESC B 0.
- `_EndPage` (`0x1000089b0`): ESC G and optionally ESC E; original calls `_FlushDataStreams` afterwards.
- `_EndDocument` (`0x100008ac0`): optional ESC E, always ESC @ and ESC Z. Original also closes the CUPS raster stream.
- `_FlushDataStreams` (`0x1000015f0`): `fflush(stdout)` and optionally `fflush(__fd_dump)`; its return values are ignored by the manufacturer executable.

**Build check:** `make clean all check` succeeded with GCC 12, `-std=c11 -Wall -Wextra -Werror`. This checks these isolated output decisions, not a complete print job.

**Still missing:** faithful integration of `_raster2printer`, `_ReadPage`, `_DitherPage`, the complete `_ProcessPage` state machine, network/PRN output, and the remaining functions and error/termination paths. The existing experimental work under `enhancements/` is kept separate because it contains deliberate changes to the original behavior.

The macOS binary has not been executed and compared for exact output. This is not a completed 1:1 reconstruction.
## Original halftone tables and PrintQuality selector — 2026-10-13

- `raster2printer_matrices_recovered.c`: preserves all **13 sets of integer matrix values** previously extracted from the vendor Mach-O data segment; the original addresses and previously recorded raw checksums remain in the array comments. All **5237** entries are transferred as data, without any changed matrix values.
- `raster2printer_quality_recovered.c`: preserves the **15 choice spellings and selector indices** previously established for original `_raster2printer` code at `0x100004dbd..0x100005098`. All 15 choice names were checked against the genuine classic PPD. Case-insensitive comparisons match the observed choice comparisons. In particular `CUPSThresold64x64` keeps its original spelling, and `Draft` is index 7.
- Selector descriptors preserve 15 original table/dimension/divisor entries, including the two references to the same 144-element matrix for modes 1 and 2. For `Draft`, the null table is a documentation sentinel: the observed binary reaches that path without an initialized matrix pointer; `Draft` uses 1-bit input in the original PPD, not RGB dithering.
- `raster2printer_quality_recovered_test.c` and the updated Makefile describe offline **compilation/check targets**. No fresh compiler execution or original Mach-O comparison has been performed in the current session; the local execution environment was unavailable. Previously recorded smoke tests are not evidence for this new revision.

These are recovered **data and option-selection fragments**. They are not a recovered implementation of the original `_DitherPage`, nor are they a drop-in binary replacement. They contain no deliberate alterations to the 13 original numeric arrays. The original Mach-O files themselves were not stored in this GitHub branch.

## Recovered _DitherPage RGB24 pixel conversion

`raster2printer_dither_recovered.[ch]` now records the **previously reverse-engineered** inner RGB24 conversion from OEM `_DitherPage` (Mach-O `0x100006dd0..0x100007338`). It uses the OEM 13 matrix tables, choice indices, floating-point divisors and bit packing without the added validation of the prior experimental code. A minimal valid-RGB24 input test is included in the existing offline Makefile.

This is a **C fragment**, not the OEM function ABI: the surrounding original memory allocation, raster header, per-page option selection, fallback and error paths are **not recovered**. The OEM `Draft` choice selects a 1-bit input route; calling this RGB24 fragment with selector 7 is outside the recovered path. The experimental source with deliberate input validation stays under `enhancements/`.

No fresh compilation, binary-reference execution or printer test was performed in this session: the local compiler/container runtime is unavailable. The prior reconstructed model is not a substitute for byte-exact comparison with the original macOS executable.


## Recovered `_ReadPage` and `_WriteData` branch fragments

This revision adds two newly authored, isolated C interfaces corresponding to **observed manufacturer branches**. Neither is a complete clone of the original Mach-O function, argument ABI or surrounding macOS/CUPS integration.

**`classic_read_page.[ch]`** — original `_ReadPage`, `0x100008d20..0x10000918b`:

- `cupsRasterReadHeader2` returns zero: EOF; nonzero: page counter increments *before* the pixel allocation or reading.
- 32-bit `cupsHeight * cupsBytesPerLine` allocation expression and one `cupsRasterReadPixels` request for the entire page.
- A zero returned pixel count selects the failure branch. A **nonzero short read is accepted**, reproducing the branch observed in assembly rather than the protective complete-read check in the enhanced implementation.
- The native page buffer layout, all header-dependent state and original ownership/error unwinding are **not** recovered. The standalone C callback interface therefore exposes allocation ownership explicitly; this interface itself is not manufacturer code.

**`classic_write_data.[ch]`** — original `_WriteData` at `0x100001450`:

- One selected `send` call for IPP/socket output; a positive short send is not retried.
- The alternate stdout path optionally writes to the PRN/debug file before stdout; the debug write return is not treated as fatal.
- The offline interface is explicitly passed a previously selected sink: **the OEM's full URI/global-file selection is not yet recovered**, and its original return-value ABI and diagnostic side effects have not been verified.

Source-only checks `classic_read_page_test.c` and `classic_write_data_test.c` cover these recovered decisions and are included in `make check`. The original programs have **not** been executed for comparison and there is **no 1:1 equivalence claim**.

## _ProcessPage modes 0/1 — extracted raster output framing

`classic_process_page_modes_01.[ch]` reconstructs the previously audited **mode 0** and **mode 1** row-submission path of the original `_ProcessPage`, observed at `0x100007633..0x100007739`, `0x100007939..0x100007a74`, `0x100007e6e..0x100007f32` and `0x10000889b..0x100008945`.

- Both modes emit one four-byte `ESC D <packed_stride> SYN` introduction, separately from the page setup.
- Mode 0 sends every complete packed row unchanged.
- Mode 1 accumulates all-zero rows and emits `ESC f <two-byte count>` before the next nonempty row; trailing empty rows remain deferred.
- The OEM length is based on each job's raster row, without any general fixed paper length. The source's 16-bit blank-row counter is based on the disassembly; what happens on extremely long overflows still needs execution against the original.

These are standalone fragments, **not** complete `_ProcessPage`: mode 2/3 branches, state carried between pages, cancellation, complete page positioning and error paths remain missing in this module. The previous enhanced full-row approximation is still under `enhancements/`. A separate test checks framing and deferred blank rows. The original Mach-O has not been run.

## _ProcessPage modes 2/3 — isolated row emission

The new `classic_process_page_modes_23.[ch]` implements the previously audited, disassembly-derived **row-level** mechanics of mode 2 and mode 3, with the original byte framing and state distinctions:

- Both accumulate empty rows as deferred 16-bit `ESC f` advances, emitted only when another nonempty row occurs.
- Mode 2 always emits the `ESC B` byte offset, trims both byte edges, emits an `ESC D` only on changed packed width (or the first nonblank row of an explicit reprint invocation), and selects `0x17` bit-run encoding **only when strictly shorter** than `0x16` plus raw trimmed row bytes. Run bytes encode 1–128 bits, most-significant-bit first.
- Mode 3 emits `ESC B` only when its left offset changes; the original mode-3 path trims the right-hand white bytes **only if the left offset is nonzero**. It always emits uncompressed `0x16` raster bytes.
- Mode-2 and mode-3 previous widths survive page boundaries in different job-cache fields. The previous mode-3 left offset, pending blank rows and first-nonblank state are reset at each page start.
- The compiler-facing state structures are **newly authored analysis interfaces**, not reconstructed original structure layouts. Caller-provided output capacity is required; this file deliberately does **not** inherit the bounds/error-handling improvements of the separate `enhancements/` version.

`classic_process_page_modes_23_test.c` covers the observed compression case, raw fallback, original mode-3 trimming asymmetry, blank feed, cached width across page boundaries, and the explicit reprint branch.

This module does **not** restore the entirety of OEM `_ProcessPage`: the original's loop-level progress/cancellation order, all memory accesses, option reading, start/end integration, all error paths, and original ABI are not covered. Its tests compare with previously audited reconstruction expectations; **the manufacturer Mach-O is not executed**. Neither functional identity nor full branch coverage may be claimed.

## Integration of the recovered active printing sequence (13 October 2026)

`classic_raster_job_recovered.[ch]` connects the separately reconstructed
manufacturer-derived functions into a **CUPS-independent job callback runner**:
StartDocument -> ReadPage -> optional RGB24 DitherPage -> StartPage ->
ProcessPage (0,1,2,3) -> EndPage -> FlushDataStreams -> EndDocument.
It retains the job-persistent mode-2/mode-3 width caches, sends the original
per-page cutting commands and uses no arbitrary fixed label length.
The isolated integration test exercises two consecutive three-row pages
under every optimization level and checks headers, trailers, per-page
flushes and cross-page cached width.

Important limits: this is **newly authored integration C**, not a verbatim
recovery of the OEM Mach-O ABI; the OEM CUPS/PPD command-line entrypoint,
macOS networking, original PRN/debug files, complete error/termination
branches and manufacturer executable comparison are not part of this module.
For RGB24, the integration assumes the original PPD's complete RGB triples,
and its callback error return convention is a reconstruction-level interface.
All exposed output bytes are derived from the previously audited original
subroutines, but output identity for complete jobs remains unproven.

No claim of a finished classic macOS native driver is made by this step.

## Native classic CUPS entry — ordinary stdout route

`classic_raster_cups_adapter.c` is a newly authored bridge from the original
macOS classic CUPS PPD/raster-filter APIs to the individually reconstructed
active print-job functions. It reads the marked manufacturer choices for
`Resolution`, `PrintQuality`, `OptimizationLevel` and `LabelCutter`,
opens the actual CUPS raster input and sends output via the original-style
single fwrite/stdout route. `DebugMode` is left inert as indicated by the
original classic disassembly; it is not replaced by Linux Mesh drawing.

The original network `DEVICE_URI`/IPP socket path is **not yet
reconstructed** in this adapter and remains disabled. The original
`PrintThroughPRN=ON` file/replay route is implemented and was verified
against the manufacturer's executed binary for the standard mono,
OptimizationLevel 2 reference fixture. This adapter is not
OEM C, is not a complete replacement for the classic original, and must
not be installed as a production filter. Full native SDK compilation and
execution results must be recorded separately.


## Additional manufacturer PRN helper recovery

`original_prn_helpers.[ch]` reconstructs the original classic macOS
`_OpenDebugFile` and `_SendDebugFileToPrinter` operations using actual
SHA-256-verified manufacturer x86_64 disassembly, rather than copying the
different (improved) portable code under `enhancements/`.

The OEM builds `/Library/Caches/DEBUG/dbg.prn`, calls
`mkdir(...,0755)`, calls `chmod(...,0755)` **only** if mkdir succeeds,
opens the file with `fopen(...,"wb")`, then calls `chmod(...,0644)`
regardless of the open result. The later send helper reopens the same
file with `fopen(...,"r")`, copies characters through individual
`fgetc`/`fputc` operations, stops on EOF or a failed `fputc`,
prints the original diagnostics and returns zero even on a failure.

Source: original raster2printer at `0x100001660..0x100001900` and
`0x100001b40..0x100001e0c`. The original command executable also
contains related helper functions at *different addresses*, which are
not silently treated as equivalent.

**Important:** `classic_original_prn_open` deliberately reproduces an
unsafe manufacturer-wide file path. It MUST NOT be called in a production
container or on an ordinary workstation. The isolated unit test sets
only a disposable temporary filename for replay; it does not invoke
the OEM hardcoded-path open routine. The state structure and injectable
output FILE argument are newly authored test interfaces; the separate
close routine has now been checked against the exact original
assembly (0x100001e10..0x100001e45), and the one-element
`fwrite(buffer,size,1,stream)` of `_WriteDebugBuffer` at
0x100001e90..0x100001ee8 was recovered and integrated.

`PrintThroughPRN=ON` is integrated in the CUPS entry. The actual native
macOS manufacturer and reconstructed runs each produced an identical
3,620-byte PRN file and an identical 3,620-byte stdout stream in the
tested case ([reference run](https://github.com/an3k/leitz-icon-linux/actions/runs/38434844921)).
This does not prove other PRN error branches or production safety.


## Additional original log / bit helpers (direct Mach-O recovery)

`original_log_helpers.[ch]` recovers `_OpenLogFile`
(0x100001910..0x100001b38), `_CloseLogFile`
(0x100001e50..0x100001e85), and `_is_bit_set`
(0x100001630..0x100001656) from direct original machine-code inspection.

The OEM log path is `/Library/Caches/DEBUG/LeitzDriverLog.log`,
constructed by `snprintf` for the directory followed by `sprintf`.
It calls `mkdir(0755)`, `chmod(0755)` only for a newly created
directory, `fopen("w")`, and `chmod(0644)` for the log even if the
open failed. The close helper calls `fclose` if non-null and clears the
global pointer; the bit helper returns an integer test result with the
x86 32-bit shift-count behavior.

The test only exercises the no-side-effect already-open and close
branches and all 32 bit positions. It deliberately does **not** write
the OEM's hardcoded global log path. The explicit C state structures
are analysis interfaces and are not proof that the entire original
Mac process has been recovered.

## Newly recovered native _ResolveDeviceURI (13 October 2026)

The original SHA-verified Leitz 5.1.465 `raster2printer` Mach-O was
disassembled on the Intel macOS runner, and the complete function
`_ResolveDeviceURI` at `0x100009a30..0x100009fd3` was inspected.
`original_device_uri.[ch]` reproduces this OEM CUPS/network sequence,
**separately from the production CUPS adapter**:

- `getenv("DEVICE_URI")`, then `cupsBackendDeviceURI(&uri)` and
  `httpSeparateURI(0, resolved, scheme, 1024, username, 1024,
  hostname, 1024, &port, resource, 1024)`.
- The original descriptor byte offsets `0x50`, `0x450`, `0x850`,
  `0xc50`, and `0x1098` are represented exactly and verified by
  compile-time assertions. Unidentified bytes remain unnamed.
- The function returns 0 for unresolved/malformed URI and 1 as soon as
  a URI is parsed, **even if socket creation or connection fails**.
- It tests for lowercase `ipp` by substring match and uses
  `socket(AF_INET,SOCK_STREAM,0)`, `gethostbyname`, and `connect`
  using a **hard-coded port 9100**, regardless of the parsed URI port.
- The original does not close or retry a failed socket. It calls
  `herror("resolv")` if DNS lookup fails and then continues into an
  unchecked host-entry dereference; this original defect is not silently
  repaired in the reconstruction.

`original_device_uri_test` is macOS-only and uses a non-network
`usb://` URI. It does not establish socket/connection parity.
The original `_CheckPrinterStatus` remains a separate, unfinished
device-status function. **No network equipment should be connected
during the original-code experiment without explicit authorization.**
The existing functional stdout/PRN driver paths remain unchanged.

## Original _CheckPrinterStatus reply classification — 2026-10-13

The native original `_CheckPrinterStatus` function at
`0x100009190..0x100009a2e` has been disassembled on the Intel macOS
runner. `original_status_decoder.[ch]` reconstructs its observed response
**decision path** separately from the unfinished full original function:

- The OEM sends `ESC U` either through a connected TCP socket or using
  stdout followed by `cupsBackChannelRead` with a 64-byte buffer.
- Text containing `SKU:Load Error;` **or** `USED:-1` selects
  `com.leitz.out-of-paper-error`, emits a four-byte reset/end command
  `ESC @ ESC Z` and sleeps 5 seconds.
- Text containing `SKU:Door Open;` or `SKU:No Cartridge;` emits the
  corresponding printer state and sleeps 5 seconds.
- `SKU:Bad Cartridge;`, empty response and read failure lead to
  `com.leitz.general-error`. The literal OEM message says it will retry,
  but the path itself does not always call `sleep(5)`.
- Other nonempty text selects `com.leitz.ready`. An apparent incompatible
  cartridge message path is unreachable from the observed default branch
  because the OEM sets and tests its local flag immediately before it.
- The helper does not introduce alternate interpretations of these reply
  strings, new case folding, or a safety-driven change in status values.

This is an isolated recovered **decision helper**, NOT the complete
`_CheckPrinterStatus`: the original socket/backchannel transactions,
retry loop, SIGTERM handling, error messages, exact descriptor layout,
timeout value, return code and original write/flush calls still need to
be connected and checked against real original execution. The helper
is included in the GCC/Clang offline test target. No status results are
inferred from the separate `command2leitz` status byte format.

## 14 October 2026: Original _CheckPrinterStatus I/O, status and retry loop

`original_status_exchange.[ch]` reconstructs **additional previously missing**
control flow from the verified classic Leitz `raster2printer` Mach-O at
`0x100009190..0x100009a2e`, using the same original manufacturer bytes as
`original_status_gate` and `original_status_decoder`. This is a standalone
function with an analysis callback interface; it is NOT linked to the CUPS
adapter and cannot contact a real printer.

- At entry, with an IPP scheme and missing socket (or termination), the OEM
  calls `_ResolveDeviceURI` then **returns immediately**; it does not
  automatically open a socket for every ordinary print job.
- Otherwise, it sends exactly `ESC U`, via a single socket `send(2)` followed
  by a `recv(64)`, or by stdout putchars followed by flush and
  `cupsBackChannelRead(64, 7.0)`. The `7.0` timeout was decoded directly
  from the manufacturer's Mach-O constant at virtual address `0x10000a4a0`.
- The original prints `STATE: none` before classifying the answer. For
  `SKU:Load Error;` or `USED:-1`, it increases job size by four, invokes
  `_WriteData` for `ESC @ ESC Z`, and sleeps five seconds. For open door
  or missing cartridge, it emits the corresponding state and also sleeps five.
  These three statuses **repeat** the status loop; no retry ceiling is added.
- A ready answer, unknown nonempty string, bad cartridge, empty reply or
  failed backchannel read exits. The original returns a bool indicating
  whether it encountered a waiting-media/door/cartridge condition; it is
  **not** a success/ready indicator.
- The original does not NUL-terminate 64-byte replies and uses an
  uninitialized read count after a socket send of less than two bytes.
  Neither undefined behavior can be equivalently mapped to deterministic C.
  The isolated analysis helper therefore reports `-2` exclusively for
  these **unverified original paths**; this is not an OEM return value or
  a production driver error policy. The original's by-value descriptor
  copy into `_WriteData` and full globals ABI are also not implemented.

The recovered branch simulation has GCC, Clang and sanitizer tests for
ready, repeated out-of-paper, door/no-cartridge with termination, empty and
failed reads, resolver entry, and a successful socket query. It has not been
directly invoked against the original Mach-O's private function; no
whole-program equality may be claimed. The new code does not modify
`enhancements/` or the active Linux/Unraid driver.

## Original signal and C++ compiler helper bodies

`original_raster_private_helpers.[ch]` implements the previously missing short native functions observed at `0x100004420`, `0x100004570`, `0x100008cf0`, `0x100009fe0`, `0x10000a010` and `0x10000a040`. The original cancellation handler writes the literal message to stderr using `fprintf` and then assigns integer 1 to `terminate_flag`. C-compatible wrappers reproduce original libc `strrchr` / `strstr` calls and 16-bit byte swap. The local test checks diagnostic bytes, flag state, string-lookup behavior and every 16-bit input.

The recovered C interfaces do not recreate original static C++ mangling or register the signal handler into the live CUPS adapter. No replacement, improvement or protection is added to the original unsafe `fprintf` signal-handler behavior. This addition alone does not establish complete executable equality.

## 2026-10-14 — verified OEM _WriteData zero-only success criterion

The manufacturer x86_64 Mach-O `_WriteData` routines were checked again at `command2leitz 0x100001a80..0x100001c14` and `raster2printer 0x100001450..0x1000015e4`. Both executables compare the primary `send` or `fwrite` result **only against zero** and return 1 for any nonzero value, including a signed `send` result of -1. Both OEM versions emit the exact error text `\nraster_c_functions.c:55:WriteData(): an error occured while writing data of size : %ld` **only** when the selected sink returns zero. On non-socket output, the original returns 0 immediately for zero length or no stdout stream, without an error message. The debug dump's return value is ignored.

The two preexisting, incomplete sink-choice C implementations were corrected **in place only on those differences**; no alternative implementations, retry loops, additional guards, or changes to the established good paths were introduced. The separate cross-executable test in `verification/macos-5.1.465-classic/original_write_data_semantics_test.c` checks signed -1, positive short sends, zero sends, zero/positive stdout writes, PRN-only output, zero-length writes, dump ordering, and complete OEM stderr bytes. Local GCC and Clang with AddressSanitizer/UndefinedBehaviorSanitizer passed. CI builds now run this cross-test on Linux GCC/Clang and macOS Clang.

The test is based on the **verified original assembly**, not on executing the original internal `_WriteData` directly. This still does not establish exact original ABI, network/device behavior or full program equivalence.

## Full original _ReadPage recovered as separate C function — 2026-10-14

Reviewed the **unchanged, SHA-256-verified manufacturer's**
`raster2printer` binary, function `_ReadPage` at
`0x100008d20..0x10000918b`. The existing simplified
`classic_read_page_core` and its callers were not modified.

New `original_read_page.[ch]` reconstructs **all defined branches**
of this original function: call to `cupsRasterReadHeader2`, incrementing
the page counter before other work, original media-type/name diagnostic
(line 1049), copying width and height into 16-bit descriptor fields,
original 32-bit output-stride calculation with division and remainder,
bits-per-pixel, bytes-per-line and color-space fields, unbounded original
`strcpy` to media name, two original line-width diagnostics (1059 and
1061), freeing a prior page buffer when a previous page exists,
32-bit page byte count, `calloc(1,count)`, **one**
`cupsRasterReadPixels` call per page, and the original outcome paths:

- Header result zero: return zero without incrementing the counter.
- **Any nonzero pixel count, including a short positive read:** report
  received/requested sizes at line 1089 and return 1.
- Pixel count zero: set requested byte count to zero, free the new
  pixel buffer if non-null, and return zero. The original **leaves a
  dangling pointer** without clearing it. This behavior is preserved
  for source fidelity, not recommended for application use.

The reconstructed descriptor uses exact manufacturer field offsets
(`0x1c`, `0x1e`, `0x20`, `0x30`, `0x34`, `0x3c`,
`0x1050`) with static layout assertions; unknown original fields
remain opaque. This does **not** establish the original C++ global or
binary ABI. No fixed label length, bounds hardening, short-read retry,
allocation check, or other behavioral modification was introduced.

Test-only CUPS declarations are isolated under
`readpage-fixture/`, excluded from native compilation.
GCC, Clang, and Clang ASan/UBSan tests passed locally with simulated
successive pages, nonzero short reads, buffer replacement, zero read,
EOF, geometry conversions and original stderr messages. Native
macOS compilation with **real Apple CUPS headers** and the existing
manufacturer comparison suite are configured in CI; their result must
be checked for this commit. The original internal function has not
been independently invoked within the OEM executable.

**Scope:** Complete defined control flow of one function, separately
reconstructed; full `raster2printer` integration and total functional
equivalence of either executable remain incomplete.

## Original _DitherPage function wrapper — 2026-10-14

The original raster2printer Mach-O `_DitherPage` at
`0x100006dd0..0x100007335` was re-read from the verified Intel binary.
It takes exactly four positional arguments: a descriptor pointer, RGB24
input bytes, 1-bit output bytes, and the OEM quality-selector integer.
Before processing any pixels it emits these **unconditional** original
stderr diagnostics:
- `raster2printer.c:593:DitherPage(): Height = %i`
- `raster2printer.c:594:DitherPage(): Bytes per line = %i`

New `original_dither_page.[ch]` reconstructs this descriptor-level
function using the original offsets `0x1c` (16-bit height) and `0x34`
(32-bit input row length). It deliberately **reuses** the previously
recovered `raster2printer_dither_recovered.c` RGB24 inner loop, original
13 raster matrices, and quality-selection data. The pixel logic was
already present and has not been copied or rewritten. The new function
adds only the formerly missing OEM field access and diagnostic sequence.

The original function accepts quality 7 through its switch but enters
the RGB24 loop with an uninitialized matrix pointer; invalid input and
non-3-aligned RGB geometry have undefined original behavior. The
defined function-level reconstruction therefore covers OEM RGB24
quality modes 0–6 and 8–14, without a new fallback, correction or
boundary check. Actual 1-bit `Draft` jobs bypass OEM dithering.

The standalone `original_dither_page_test.c` proves exact OEM
diagnostic bytes and argument/geometry transfer via a test-only fake
pixel core. The independently tested actual OEM-derived pixel core
remains linked to the real `original_dither_page.c` in normal
compilation; no second implementation is created. GCC/Clang local
builds passed. Native macOS CUPS SDK compilation and the current
complete original-printing comparison suite are run in CI. The
original private `_DitherPage` has **not** been invoked directly.

Previous commit `f1cb4cae81b51499c37c2d4dbda4d93ffb56395f`
successfully completed all three GitHub Actions jobs, including native
Apple CUPS compilation and manufacturer reference comparisons. These
results apply to the recovered `_ReadPage` function; newer changes
must receive their own completed run.

Neither this wrapper nor the newly reconstructed `_ReadPage`
is connected to the active `classic_raster_cups_adapter`. Its
continued use of the older simplified core remains unchanged.

## OEM raster main entry recovery — 2026-10-14

The previously unfinished original `raster2printer` `_main` function
was independently disassembled at `0x100004460..0x100004565` from the
SHA-256-verified Leitz 5.1.465 classic Intel Mach-O. The new
`original_raster_main.c` translates the **complete defined entry call
sequence** directly rather than introducing callback behavior:

1. `_OpenLogFile` unconditionally, with its original return ignored;
2. `fprintf(stderr, "\nraster2printer.c:41:main(): raster filter started")`;
3. reset the original global integer `terminate_flag` to zero;
4. register the native `SIGTERM` handler and ignore `SIGPIPE`;
5. reset original integer `job_size` to zero;
6. invoke original `raster2printer(argc,argv)` once;
7. unconditionally call `_CloseLogFile` and return the filter's result.

No argument validation, error conversion, additional cleanup or safer
signal behavior was added. A native `main` can be enabled by defining
`CLASSIC_OEM_FINAL_MAIN` only when the remaining manufacturer functions
are all linked; the existing separately tested CUPS adapter is unchanged.
The original C++ symbol/linker and global storage arrangement are
not claimed complete.

`original_raster_main_test.c` verifies exactly the start diagnostic
bytes, `OpenLogFile` / `raster2printer` / `CloseLogFile` order, original
`SIGTERM`/`SIGPIPE` dispositions, global resets and unchanged
result propagation for multiple return codes. GCC, Clang and
ASan/UBSan passed locally. The sources-only CI will run these
additional tests under both OSes.

At earlier commit `dc0c92aa680d7bd26eea60e40f5615467d4de069`,
[run 38492636976](https://github.com/an3k/leitz-icon-linux/actions/runs/38492636976)
completed all three jobs, including native macOS CUPS SDK
compilation of the newly reconstructed `_DitherPage` and vendor
reference tests. This confirms compilation and checked scenarios;
it does **not** establish full private-function or executable parity.

The active `_raster2printer` function remains an important
unfinished reconstruction. The new `_main` is not yet a complete
runnable substitute for the manufacturer binary.
