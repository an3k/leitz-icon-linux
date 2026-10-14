# macOS 5.1.465 classic — reconstruction state

## Manufacturer inputs (unmodified)

- `raster2printer`: Mach-O x86_64, 84,252 bytes, SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`.
- `command2leitz`: Mach-O x86_64, 35,932 bytes, SHA-256 `6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee`.
- `LeitzIcon.ppd`: unmodified manufacturer PPD.
- The original archive does **not** contain original C files for either executable.

## C source availability

**Neither executable has a complete faithful C reconstruction.** The existing symbol lists and assembly analyses describe specific functions but cannot establish the original C source, all code paths, or full binary-to-C equivalence.

Experimental, compilable C fragments formerly kept in `reconstructed/` have been moved unchanged into `enhancements/macos-5.1.465-classic/`. Do not attribute these files to the manufacturer or use them as exact replacements.

The fragment library, command filter and test executables were built using GCC on Linux (`make clean all check`, successful); the full CUPS raster adapter could not be compiled in the local environment because genuine CUPS development headers are unavailable. The original Mach-O executables have not been run for reference-output comparison.

## Documented intentional divergences in the experiments

- Rejecting stdout/PRN flush errors where the original routine does not check the return code.
- Extending the portable raster-header callback to distinguish negative errors from end-of-file.

Further differences may exist. Neither the successful builds nor local smoke tests establish equivalent printing or cutting behavior.

**Before a 1:1 claim:** reconstruct every active code path for both executables; compile for macOS using compatible system APIs; compare identical inputs/options, device-output bytes, status exchanges, errors and edge conditions against the original executable. No such final comparison is available.

## Additional recovery committed on 2026-10-13

- `reconstructions/macos-5.1.465-classic/command2leitz/command2leitz_recovered.c` plus `command2leitz_cups.c`: independently recovered active command-dispatch and CUPS entry path, built and exercised offline. Original execution and full macOS equivalence remain outstanding.
- `reconstructions/macos-5.1.465-classic/raster2printer/`: five raster lifecycle/output boundary functions reconstructed separately from assembly; built and checked offline. The raster filter as a whole remains incomplete.
- Existing experimental C stays isolated under `enhancements/`, and the manufacturer originals remain unmodified.

## 2026-10-13 — recovered manufacturer halftone values

- All 13 matrix tables and all 5237 integer elements previously extracted from the classic Mach-O are now present in standalone C under `reconstructions/macos-5.1.465-classic/raster2printer/`. C declarations expose these arrays without changing their numeric values.
- All 15 original `PrintQuality` labels were matched against the byte-identical original PPD, and their previously reverse-engineered selector indices, matrix sizes and scaling constants are in separate C.
- Full `_DitherPage` and full raster filter remain incomplete; no 1:1 equivalence claim. This revision has **not** been newly compiled because the container runtime was unavailable.

## 2026-10-13 — dither inner loop reconstruction

Added `reconstructions/macos-5.1.465-classic/raster2printer/raster2printer_dither_recovered.[ch]`, extracted from the already documented OEM `_DitherPage` RGB24 arithmetic. The recovered fragment uses manufacturer matrix values and exact previously decoded mode selection, without previous experimental boundary checks; additional test inputs have been added to the offline Makefile. **The complete original function is not recovered, and no compiler or OEM execution check was possible in the current session.**

## 2026-10-13 — raster input and output recovery

New standalone C in `reconstructions/macos-5.1.465-classic/raster2printer/`: `classic_read_page.[ch]` and `classic_write_data.[ch]` with explicit callback boundaries. These correct two differences from previously hardened project code: page index increments at header acceptance and nonzero short page reads remain successful; short socket sends are not automatically retried. Original branching evidence is documented by original Mach-O virtual addresses. No full ABI, complete job path or binary-equality assertion is made. GCC/Clang CI build verification is configured separately and must not be reported as passing until the workflow concludes successfully.

## 2026-10-13 — ProcessPage modes 0/1 independent recovery

Added `reconstructions/macos-5.1.465-classic/raster2printer/classic_process_page_modes_01.[ch]`: original 0/1 raster row header and row forwarding, including the original deferred empty-row handling in mode 1. The standalone ABI is project-authored; a full equivalent `_ProcessPage` including 2/3, global state and all branches is **not yet available**. GCC/Clang verification is part of the sources-only workflow and must be checked against the completed run for the associated commit.

## Verified source build: 2026-10-13

For commit `1345811cea4198e52554e8f3e0ac0738ceea2fd3`, workflow [Sources - Reconstruction Compile](https://github.com/an3k/leitz-icon-linux/actions/runs/38379977044) **completed successfully**: GCC and Clang built and exercised both isolated command and raster recovery test suites. This confirms **compilation and local test results only**, not executable parity with the original macOS Mach-O files or hardware behavior.

## 2026-10-13 — mode 2/3 row path recovery

Added `reconstructions/macos-5.1.465-classic/raster2printer/classic_process_page_modes_23.[ch]`, based on the classic OEM `_ProcessPage` branches at `0x100007a74..0x100008882`. It recovers row-level original mode-2 bit runs, strictly shorter compressed selection, mode-3 edge treatment, dynamic offsets/widths, deferred blank rows, page-persistent width caches, and the distinct explicit reprint exception. The existing mode 0/1 source and `enhancements/` code were not changed. The full manufacturer function and original Mac binary parity remain unresolved. A GCC/Clang build workflow is triggered on this new commit; record actual outcome separately.

## Verified build — modes 2 and 3

For commit `86af3344029ed28d52fea915a453f065f5bdb3da`, the [Sources - Reconstruction Compile](https://github.com/an3k/leitz-icon-linux/actions/runs/38382200525) workflow completed successfully: the recovered classic command and raster fragments, including the newly added 2/3 row paths, built and passed their offline checks under **GCC and Clang**. The check compares known, disassembly-derived byte sequences; it is **not** an execution of the vendor Mach-O or a binary equivalence test. No unchanged manufacturer C files were modified.


## 2026-10-13 — measured OEM runtime parity (supersedes historical pending-test statements above)

The **actual original Leitz 5.1.465 x86_64 Mach-O programs**, downloaded
from Leitz's official installer and verified against the exact SHA-256 hashes
at the beginning of this file, were run on an Intel macOS-15 runner against
the separate recovered C executables. These comparisons do not involve a
physical printer.

- Original `raster2printer` vs recovered filter with **14 original RGB24
  `PrintQuality` choices × 2 resolution modes × 4 optimization modes =
  112 tested combinations**: all 112 emitted exactly the same stdout
  bytes in the tested two-page fixture. Original `Draft` uses the 1-bit
  path rather than RGB24.
- Two-page monochrome printing: for optimized modes 2 and 3 exact stdout
  parity; for unoptimized modes 0 and 1, the OEM sends **four additional
  bytes beyond every raw row's nominal stride**. The recovered source now
  reproduces the read size. For these modes the last four bytes of each page
  are undefined OEM memory, so comparison of **all defined bytes** is
  reported separately and must never be called universal exact parity.
- Four additional combinations with 33 and 127 raster rows, modes 2 and 3,
  `LabelCutter=OFF`: exact emitted bytes, including original end-of-job
  cutter behavior. There is no imposed fixed label length.
- Original `command2leitz` vs CUPS entry: seven cases covering recognized
  status/firmware queries, empty or failed backchannel response and an
  unrecognized command. A real macOS dynamic interposer supplies synthetic
  read responses to the original; the same responses are supplied to
  recovered code. Exit codes, raw stdout and `STATE:/ATTR:` lines match
  across every exercised case.
- `_OpenDebugFile` and `_SendDebugFileToPrinter` reconstructed from
  native original disassembly, including original mode bits 0755/0644
  and `fopen("wb")` / `fopen("r")`. The recovered native
  `PrintThroughPRN=ON` output path was then compared against the actual
  original: both produced an identical 3,620-byte intermediate PRN file
  and an identical 3,620-byte stdout stream for the verified test case.
- GCC, Clang, Intel macOS test builds and native macOS SDK CUPS adapter
  builds passed. Executions, not merely compilation, ground the comparisons.

Verified logs:
- [Command filter reference run](https://github.com/an3k/leitz-icon-linux/actions/runs/38431824113)
- [First 32 RGB24 cases](https://github.com/an3k/leitz-icon-linux/actions/runs/38432253626)
- [Remaining 80 RGB24 cases and dynamic lengths](https://github.com/an3k/leitz-icon-linux/actions/runs/38432927269)
- [Original PRN file transfer](https://github.com/an3k/leitz-icon-linux/actions/runs/38434844921)

**Not yet complete or proven equivalent for all input/control paths:**
the native `_ResolveDeviceURI` and `_CheckPrinterStatus` branches,
real IPP socket setup, original global logging and diagnostic helpers,
exception/termination behavior under all causes, full original function
ABI/layout and real-device communication. The test helper structures
are newly authored, and actual manufacturer C source was absent from
the installer. Neither the OEM's source code nor whole-executable
behavior has been reproduced in full. The safer alternative modules
under `enhancements/` remain separate.

## Continued primary-source recovery on 2026-10-13 (current)

The branch was inspected before this continuation: prior work was already
present through `08b468f4` and was **not** rewritten or repeated.

New direct manufacturer disassembly, obtained from the official 5.1.465
installer on the Intel macOS reference runner:

1. `_ResolveDeviceURI` `0x100009a30..0x100009fd3` is represented
   independently as `reconstructions/macos-5.1.465-classic/raster2printer/original_device_uri.[ch]` using
   original descriptor offsets, `cupsBackendDeviceURI`,
   `httpSeparateURI`, IPv4 socket setup and **fixed TCP port 9100**
   regardless of URI port. The original keeps a socket failure and null
   DNS pointer defect; these have not been silently improved.
2. `_CheckPrinterStatus` `0x100009190..0x100009a2e` was disassembled.
   `reconstructions/macos-5.1.465-classic/raster2printer/original_status_decoder.[ch]` now records its
   original string classification, `ESC @ ESC Z` empty-label action
   and branch-dependent five-second delays. It is **only** the
   classifier, not the whole CUPS/socket/status/retry function.
3. The original `command2leitz` was actually executed with four
   additional synthetic ESC A responses: `MEDIA_EMPTY`, `MEDIA_JAM`,
   `GENERAL_ERROR` and an unknown value. Stdout and state-message
   comparisons were successful for those cases. The authoritative
   manufacturer program was verified by original SHA-256 prior to running.
4. The original `raster2printer` was executed with
   `DEVICE_URI=ipp://127.0.0.1/ipp/print` while an isolated
   local-only listener monitored TCP port 9100. For this **particular
   two-page print input**, it returned success and produced the normal
   3,620-byte stdout stream but established **no TCP connection**.
   Consequently the experimental CUPS adapter must **not** be changed
   to connect to a socket merely on the basis of the environment URI.
   The native resolver exists in the original binary but its normal
   activation conditions require further source tracing.

Evidence:
- [Native expanded status and assembly run](https://github.com/an3k/leitz-icon-linux/actions/runs/38461648480)
- [Original URI build and macOS verification](https://github.com/an3k/leitz-icon-linux/actions/runs/38462879963)
- [Actual localhost-only OEM network observation](https://github.com/an3k/leitz-icon-linux/actions/runs/38463372694)

Remaining gaps have **not** been reclassified as completed:
complete `_CheckPrinterStatus` control flow and native caller integration,
precise network activation and socket lifetime in all cases, remaining
original helper/error/signal paths, whole-program ABI/layout correspondence,
and real-device output. No complete functionally equivalent replacement
for both original macOS executables has been established.

## Verified continuation after latest-state inspection — 13 October 2026

Before making changes, branch `sources` was checked at `8d0997704ecf314d19dcfa7ddd8dee20842c4a8b`. The already completed 112 RGB24 scenarios, native status commands, deferred PRN and variable-length comparisons were not duplicated or overwritten.

- Actual `nm -nm` original x86-64 symbol inventory is now kept in
  [ORIGINAL_SYMBOL_INVENTORY.md](ORIGINAL_SYMBOL_INVENTORY.md): 31 entries for
  `raster2printer` and 20 for `command2leitz`, including entry and
  compiler/library helper symbols. This is a source/function inventory,
  not a completion percentage.
- Newly reconstructed `reconstructions/macos-5.1.465-classic/raster2printer/original_status_gate.[ch]`
  reproduces the **original early _CheckPrinterStatus resolver-entry
  decision** (`0x1000091c8..0x100009243`) from verified manufacturer
  disassembly. It is independent from the previously recovered answer-text
  classification and does not claim the remaining status/socket/retry loop.
- GCC, Clang and Intel macOS compiler/test jobs plus the original macOS
  reference execution **all passed** in
  [workflow 38467570955](https://github.com/an3k/leitz-icon-linux/actions/runs/38467570955).
- No manufacturer Linux or macOS original binary files or enhanced modules
  were modified by this continuation.

**Still not complete:** the remaining original function bodies including
`_output_cups_info`, `_test_raster`, `_PrintPageDebugInfo` for both
executables, complete OEM status-check/socket state logic, native function
ABIs, all error paths and comprehensive reference equality for all inputs.
Therefore a fully reconstructable 1:1 C replacement cannot yet be declared.

## 2026-10-14 — latest-state checked, native command helpers reconstructed

Before writing, branch `sources` was independently checked at `32bae496d3952985cffd28a6a307cc233047d157`. The previously proven 112 RGB24 comparisons, command response cases, PRN transfer, and variable-length jobs were left unchanged. No original manufacturer files or optimized `enhancements/` were overwritten.

A fresh, SHA-256-verified macOS Intel reference run captured the actual `command2leitz` machine instructions. Four previously unimplemented helpers now have isolated C in `reconstructions/macos-5.1.465-classic/command2leitz/classic_command_original_utils.[ch]`:

- `_is_bit_set` at `0x100001c60`: original 32-bit bitmask comparison, including x86 shift-count masking;
- `_FlushDataStreams` at `0x100001c20`: unconditional output flush and optional debug flush, with error results ignored;
- `_CloseDebugFile` at `0x100002440`: conditional fclose and clear the file pointer;
- `_CloseLogFile` at `0x100002480`: conditional fclose and clear the file pointer.

The callback/pointer interfaces are separate reconstruction scaffolding and **not the original ABI**. The original command `_WriteData` and other longer functions remain incompletely reconstructed. Passing source compilation alone does not establish whole-program equivalence or hardware readiness.

## 2026-10-14 — command2leitz output writer recovered

New `reconstructions/macos-5.1.465-classic/command2leitz/classic_command_write_data.[ch]` records manufacturer `_WriteData` (original command Mach-O `0x100001a80..0x100001c14`), whose machine instructions were captured from the SHA-confirmed official macOS installer in [reference run 38479552118](https://github.com/an3k/leitz-icon-linux/actions/runs/38479552118). The independently isolated implementation preserves the original socket selection (URI contains `ipp` and socket != -1), one `send` without retry, optional debug-file `fwrite`, conditional stdout `fwrite`, and acceptance of positive short writes. The diagnostic `fprintf` failure text and original global ABI remain incomplete; the recovered wrapper is not a byte-identical original source file. A separate C smoke test validates the sink decisions without opening a network connection or modifying the existing command entry.

All GCC, Clang, native macOS and OEM-reference CI jobs for [run 38479996800](https://github.com/an3k/leitz-icon-linux/actions/runs/38479996800) passed. This validates compilation and the existing reference scenarios, **not the untested writer's original socket error diagnostics**. Do not report the whole original executable as reconstructed.

## 2026-10-14 — authentic macOS classic test-raster reconstruction

After checking the latest branch `sources` state, both archived manufacturer Mach-O binaries were extracted directly from the original uploaded macOS installer and SHA-256 verified: raster2printer `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`, command2leitz `6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee`.

Their previously missing dormant `_test_raster` functions were independently disassembled in full at `0x100003c60..0x100003e40` and `0x100004290..0x100004470`; both have the same observed control flow. Newly recovered source under `reconstructions/macos-5.1.465-classic/shared/diagnostics/original_test_raster.c` preserves the genuine CUPS raster-open/header/one-row-per-read/close sequence, native source-line diagnostic and copy count, original zero-only pixel-read exit and absent extra allocation/short-read guards. The CUPS mock is strictly isolated to `fixture/` and not used in the macOS production build.

The isolated GCC/Clang offline tests and AddressSanitizer/UBSan test passed locally. A native macOS SDK compilation and CI run are configured; their outcomes must be checked after the commit. No change was made to normal raster output, either original manufacturer file, or any `enhancements/` source. The new code **does not complete** either original executable and has not been compared by directly invoking the dormant function inside the original Mach-O.

## 2026-10-14 — completed recovery of both dormant _PrintPageDebugInfo functions

Both authentic binaries were disassembled directly (same verified SHA-256 identities recorded above). Raster executable `0x100003e50..0x100004419` and command executable `0x100004480..0x100004a49` contain equivalent original `_PrintPageDebugInfo` functions. The seven actual OEM `fprintf` format strings, seven source line numbers (385–389, 391, 392), original source pathname basename, CUPS header field selection, floats and unbounded `%s` string output are reconstructed in `reconstructions/macos-5.1.465-classic/shared/diagnostics/original_page_debug.c` without deliberate enhancements.

GCC and Clang have compiled the new source locally and its exact-text offline test passed. macOS SDK compilation is configured and must be confirmed from the completed Actions run. Neither the OEM executable nor the ordinary printer/command filter is modified; the helper is dormant. This completes one more individually verified native function, **not** the complete source-reconstruction project.

## Verified compile and reference job for the recovered diagnostic functions

At commit `867492daeb82a0e1579f18092003240d4671355d`, [Sources - Reconstruction Compile (run 38482664031)](https://github.com/an3k/leitz-icon-linux/actions/runs/38482664031) finished with **all three jobs successful**: GCC/Clang isolated tests, original-derived macOS C tests and genuine macOS CUPS SDK compilation of `original_test_raster.c` and `original_page_debug.c`, and the existing original-program reference scenarios. Local AddressSanitizer/UndefinedBehaviorSanitizer checks for both individual recovered diagnostic helpers also passed.

This confirms the reconstructed dormant helpers compile and emit the documented outputs in isolation; it does **not** prove that the original dormant OEM functions were directly executed against matching inputs, nor does it establish whole-program behavior equivalence. The active print or command entrypoints and original Linux/macOS manufacturer files were not modified.

## 2026-10-14 — CheckPrinterStatus native I/O and retry recovery

Inspected branch `sources` at `1df6fd4bbbfd8acbdd9249ddbec9ed35117fd432`
before starting; the existing vendor parity comparisons and the previous
status gate/decoder were left unchanged.

A newly isolated `reconstructions/macos-5.1.465-classic/raster2printer/original_status_exchange.[ch]` recovers
the original `_CheckPrinterStatus` active I/O, retry, return-value and
termination paths from the verified Mach-O code at
`0x100009190..0x100009a2e`. The original CUPS timeout is **7.0 seconds**,
decoded from the native constant. Offline GCC, Clang and sanitizer unit
tests have passed locally. The GitHub Actions build must be confirmed
after committing.

**Remaining limitations:** the original by-value descriptor/globals ABI,
socket-short-send and full 64-byte uninitialized memory cases, exact network
activation through the caller, ancillary diagnostics and native whole-program
parity. The new function is a standalone reconstruction and is not connected
to the production print path.

## Verified original-source CI for status exchange

The new `original_status_exchange` branch simulation at commit
`e015b441ac7eed887005318b30503a1de0e7fc36` passed all three jobs
in [Sources - Reconstruction Compile 38483579695](https://github.com/an3k/leitz-icon-linux/actions/runs/38483579695):
Linux GCC and Clang compilation plus the isolated I/O state tests,
native macOS C tests and SDK CUPS builds, and the existing authenticated
original Mac executable printing/command comparisons. The native original
`_CheckPrinterStatus` function itself has **not** been directly called
against the new simulated I/O scenarios; there is no claim of complete
byte-for-byte status-function or whole-executable equivalence.

## 2026-10-14 — complete isolated original _output_cups_info function reconstructed

This work began with a fresh branch inventory after the `sources` layout change at `cdd8b40605ff2f59f61422be061a7c1ec314fe3d`. No existing C reconstruction, OEM originals, enhancements, or printer execution path was overwritten.

The **original Linux 1.0.3 manufacturer C implementation** of `output_cups_info` in `originals/linux-1.0.3/raster_c_functions.c` was audited statement-for-statement against both verified original macOS classic Mach-O bodies:
- raster2printer `0x100001ef0..0x100003c60`
- command2leitz `0x100002520..0x100004290`

Both original functions contain exactly 1372 disassembly records, 108 calls and 140 literal references. Their PPD field access, conditionals, filter/attribute/group/option/choice loops, paper geometry/size output, format strings and final `ppdClose` are structurally the same as the Linux manufacturer C. The recovered C body is taken from the manufacturer Linux code, not invented independently. A `#line` directive and four diagnostic line constants restore the macOS OEM's actual source-line numbers (205,214,244,248,259,267,271,277,289,296,311,312,315,319,322,340).

**No improvements or safety guards added.** In particular the unusual string literal formatting, `sprintf` fixed offsets, omitted null/length checks and original resource lifetime remain. The only added header and source wrappers exist to compile this standalone function against CUPS.

`reconstructions/macos-5.1.465-classic/shared/diagnostics/original_output_cups_info.c` is complete for this isolated function. The neighboring `ppd-fixture/` mock is test-only and never included in native macOS builds. Local GCC and Clang passed tests checking OEM diagnostics, PPD filters, attrs, groups, options, choices and paper dimensions. The exact original isolated function has not been invoked inside a running OEM Mach-O; whole-program parity remains unproven.

## 2026-10-14 — original raster signal and compiler helper recovery

Disassembled the six previously unfinished, very small C++ functions from SHA-256-verified OEM macOS classic `raster2printer`:
`__Z14signal_handleri` (`0x100004420`), C++ `strrchr` / `strstr` wrapper calls and their libc delegation (`0x100004570`, `0x100008cf0`, `0x10000a010`, `0x10000a040`), and the 16-bit byte swap (`0x100009fe0`).

New `original_raster_private_helpers.[ch]` preserves the original cancellation message, integer flag assignment, search functions and 16-bit swap. This is a C-compatible functional reconstruction; the original C++ ABI/mangled static helper symbols, original globals layout and signal registration in `_main` are not claimed to be binary-identical. Deliberately, the original `fprintf` inside the signal callback is not replaced with a signal-safe logging method, since that would change the OEM implementation. GCC, Clang and ASan/UBSan tests passed locally, including all 65,536 swap inputs. GitHub source CI will compile and check the new standalone test after the commit; its outcome must be verified. No production driver integration or manufacturer source modification.

## Verified reference and source builds — 2026-10-14

- [Run 38487010945](https://github.com/an3k/leitz-icon-linux/actions/runs/38487010945) for commit `8f3aabd67ffb4420f47daedcd815c376b6ec3752`: all 3 jobs succeeded. This covered GCC/Clang isolated PPD diagnostic tests, native macOS SDK compilation of the recovered `_output_cups_info` function, and existing authentic vendor comparisons.
- [Run 38487275493](https://github.com/an3k/leitz-icon-linux/actions/runs/38487275493) for commit `a76914bacf3091c84393d8fec1d8b935921911f2`: all 3 jobs succeeded, including local raster signal/C++ helper checks on GCC/Clang and native macOS alongside the existing vendor comparison suite.

Both new OEM-derived parts remain **isolated reconstructions**; the genuine manufacturer's dormant `_output_cups_info` function and the original native signal handler have **not** been directly invoked and compared. These passing workflows do not establish full executable equivalence, and the incomplete original raster/command entry paths remain outstanding.

## 2026-10-14 — four original command2leitz file helper functions recovered

After checking branch `sources` at `88fddb5d0fc5690de6e934720456f522df9b6a85`, four remaining original command executable helper bodies were reconstructed directly from the SHA-256-verified Mach-O `command2leitz`: `_OpenDebugFile`, `_OpenLogFile`, `_SendDebugFileToPrinter` and `_WriteDebugBuffer`. New code is under `reconstructions/macos-5.1.465-classic/command2leitz/original_command_file_helpers.[ch]`. It keeps the original debug/log paths, permissions, byte-transfer loop, diagnostic strings, return values and unchecked original operations. Existing `_CloseDebugFile`, `_CloseLogFile` and `_FlushDataStreams` reconstructions were not changed or duplicated.

Standalone GCC, Clang and ASan/UBSan local tests passed for existing-open branches, exact PRN bytes and original diagnostics. The manufacturer's fixed `/Library/Caches` open path is **not run** in those tests. Linking the isolated helper functions into a complete OEM-equivalent command executable and comprehensive edge-condition comparison remain open; subsequent CI must be checked independently.

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

## Verified original _main entry parity — 2026-10-14

[Sources - Reconstruction Compile, run 38490145631](https://github.com/an3k/leitz-icon-linux/actions/runs/38490145631) **completed successfully in all three jobs** for commit `8f305a55c181b7d6590bf0df4e2191b1e8712ba9`. Besides GCC/Clang offline mock tests and macOS native compilation, the macOS vendor-reference job executed the SHA-256-verified original `command2leitz` and the separate CUPS candidate implementing `original_command_main_flow`.

The direct comparison logged **EXACT_EXIT_STDOUT_STDERR_PARITY=YES** in all four defined cases:
- `file_unknown` — valid PPD and command file containing an unrecognized command;
- `stdin_unknown` — input read through stdin with valid PPD;
- `missing_file` — failed command-file open;
- `missing_ppd` — failed PPD open.

No changes were made to existing vendor comparison scripts, original Linux sources, experimental enhancements or the older CUPS adapter.

**Scope of evidence:** the original entry's non-device pre-status path, defined errors, exact printed diagnostics and those four scenarios. Status callbacks, actual device exchange, undefined OEM memory cases and whole-program equivalence remain incomplete.

## 2026-10-14 — verified OEM _WriteData zero-only success criterion

The manufacturer x86_64 Mach-O `_WriteData` routines were checked again at `command2leitz 0x100001a80..0x100001c14` and `raster2printer 0x100001450..0x1000015e4`. Both executables compare the primary `send` or `fwrite` result **only against zero** and return 1 for any nonzero value, including a signed `send` result of -1. Both OEM versions emit the exact error text `\nraster_c_functions.c:55:WriteData(): an error occured while writing data of size : %ld` **only** when the selected sink returns zero. On non-socket output, the original returns 0 immediately for zero length or no stdout stream, without an error message. The debug dump's return value is ignored.

The two preexisting, incomplete sink-choice C implementations were corrected **in place only on those differences**; no alternative implementations, retry loops, additional guards, or changes to the established good paths were introduced. The separate cross-executable test in `verification/macos-5.1.465-classic/original_write_data_semantics_test.c` checks signed -1, positive short sends, zero sends, zero/positive stdout writes, PRN-only output, zero-length writes, dump ordering, and complete OEM stderr bytes. Local GCC and Clang with AddressSanitizer/UndefinedBehaviorSanitizer passed. CI builds now run this cross-test on Linux GCC/Clang and macOS Clang.

The test is based on the **verified original assembly**, not on executing the original internal `_WriteData` directly. This still does not establish exact original ABI, network/device behavior or full program equivalence.

## Original WriteData comparison build verified — 2026-10-14

Commit `89c8034fbe5e589d7e65d2b19d2203f203485d36` was validated by
[Sources - Reconstruction Compile, run 38491401611](https://github.com/an3k/leitz-icon-linux/actions/runs/38491401611).
**All three jobs completed successfully**: GCC and Clang compiled and
ran the new cross-executable `_WriteData` regression test,
macOS Clang compiled and ran that same test, and the original manufacturer
runtime comparison suite completed successfully against SHA-256-verified
macOS binaries.

The signed -1 socket case and exact OEM error message are supported by
the original **disassembly** and verified with isolated C tests, not by
calling the original private `_WriteData` routine with an injected failed
socket. The ordinary OEM output cases were checked by the existing runtime
comparison suite. Whole-executable functional equivalence remains open.

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

## 2026-10-14 — verified source builds for recovered ReadPage, DitherPage and raster main

- [Run 38492343344](https://github.com/an3k/leitz-icon-linux/actions/runs/38492343344), commit `f1cb4cae81b51499c37c2d4dbda4d93ffb56395f`: **all three jobs successful** for the additional original `_ReadPage` function, native macOS CUPS compilation and existing full vendor comparison suite.
- [Run 38492636976](https://github.com/an3k/leitz-icon-linux/actions/runs/38492636976), commit `dc0c92aa680d7bd26eea60e40f5615467d4de069`: **all three jobs successful** for the `_DitherPage` OEM wrapper and existing halftone core, with native macOS CUPS compilation and vendor suite.
- [Run 38492882650](https://github.com/an3k/leitz-icon-linux/actions/runs/38492882650), commit `66a89e4f23b9f14db20aee69ba20ab136d0e9094`: **all three jobs successful** for the original `_main` call order / signal / diagnostics tests, native macOS and vendor suite.

These tests prove that the new source builds on both operating systems, its defined isolated behaviors match the recovered disassembly, and **pre-existing** original manufacturer printing scenarios continue to pass. They do **not** directly execute OEM private `_ReadPage`, `_DitherPage` or `_main` against matching injected inputs. The complete native `_raster2printer` body, device communication state, additional error paths and integrated manufacturer ABI remain unfinished. No code from `originals/`, the older active CUPS adapter or `enhancements/` was altered by these changes.

## 2026-10-14 — classic OEM PPD choice recovery (new, isolated)

The SHA-256 verified original macOS 5.1.465 `raster2printer` Mach-O was inspected again at `0x1000045a0..0x100005412`, including the option branches `0x100004c74..0x1000052d1`. The existing project CUPS adapter and all previously recovered functions were retained unchanged.

`original_raster_ppd_options.[ch]` recovers the six actual `ppdFindMarkedChoice` decisions and their original call order: `Resolution`, `PrintQuality`, `DebugMode`, `PrintThroughPRN`, `OptimizationLevel`, `LabelCutter`. Their original behavior is preserved:

- OEM first zeroes a `0x10a8`-byte descriptor and sets optimization to **1**.
- `Resolution` uses case-insensitive comparison with `600dpi`, and the previously recovered 15 `PrintQuality` names use case-insensitive comparisons with the same indices. Unknown quality leaves the descriptor's current value unchanged.
- `DebugMode` compares `Mesh` → 2, `Debug` → 1, `PrintToFile` → 3, **case sensitively**. Other choices leave the existing value unchanged.
- `PrintThroughPRN` and `LabelCutter` accept only exactly `ON`; the comparison is case-sensitive.
- `OptimizationLevel` uses `atoi`, compares the resulting 32-bit value **unsigned** against 4, and maps invalid/negative/out-of-range values to zero.
- The original internal option field offsets are documented and used: `0x08` (PRN), `0x10` (cut), `0x14` (resolution), `0x18` (debug), `0x40` (optimization), and `0x4c` (quality). Fields outside this recovered fragment are untouched.
- Original `fprintf` messages at source lines **142 and 249** are emitted without adding buffer truncation or replacement logging.

**Important original platform difference:** The examined classic macOS `ppdFindMarkedChoice` sequence does **not** query `ContinuousMedia`, `ContinuousPaperLength`, or `ShouldCorrectMargins`. These names occur in the original **Linux** code but must not be silently attributed to the macOS 5.1.465 function.

The isolated source uses a small callback solely to inject `ppdFindMarkedChoice` results. Original CUPS option-array traversal, marking conflicts, and the remaining `_raster2printer` body have **not** been reconstructed into this helper, and there is no claim that the OEM entire function has become equivalent. The existing recovered `classic_raster_quality_selector` is reused by the production fragment; its name-to-index mapping already has its own tests. The isolated test uses a stub of that mapping only to verify caller semantics.

GCC, Clang, and Clang ASan/UBSan builds and local unit tests **passed**. Cases include all 15 quality choices, absent choices, exact diagnostic strings, case sensitivity, negative optimization, field offsets, and original default initialization. The sources GitHub workflow will build the new tests on Linux and macOS; its result must be checked separately.

## Verified classic PPD option recovery build — 2026-10-14

At commit `5b0a80c1ab2e390563ac2a7edf26f49416d4adcc`,
[Sources - Reconstruction Compile, run 38494696509](https://github.com/an3k/leitz-icon-linux/actions/runs/38494696509)
finished with **all three jobs successful**. GCC and Clang compiled and ran the
new isolated PPD selector test (including all 15 original halftone labels,
six manufacturer option lookups, case sensitivity, unsigned optimization
fallback, descriptor fields and exact option diagnostics). Native Apple macOS
also compiled and executed these tests. The previous original-vendor execution
comparison suite completed without regression.

The new PPD-choice fragment has **not been executed inside the original OEM
binary with injected choices** or integrated into the full
`_raster2printer` function; this is confirmed **isolated reconstruction and
test completion**, not a statement of whole-binary functional identity.
Original manufacturer's files and the current driver were not changed.

## 2026-10-14 — CUPS raster preflight recovery, original Mach-O

Checked current `sources` head `a1e5691b6a8da6f51b9c5c81acebe281ae456443` before modifying the repository. The previous PPD-choice selector and all other reconstructed files are unchanged.

New `original_raster_preflight.[ch]` translates the **previously missing CUPS/PPD entry region** of the genuine macOS 5.1.465 `_raster2printer` (`0x100004608..0x100004c74`) from direct disassembly of the verified x86-64 Mach-O (`raster2printer` SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`).

Original branch behavior recovered:
- `setbuf(stderr,NULL)`, then `argc < 6 || argc > 7` with exact diagnostic line **76** and early return **1**;
- when `argc == 7`, diagnostic line **83**, `open(argv[6],O_RDONLY)`, and on -1 diagnostic line **86**, `sleep(1)` and return **1**;
- otherwise input fd 0; `cupsRasterOpen(fd,CUPS_RASTER_READ)`, with original `perror` and return **1** if null;
- `ppdOpenFile(getenv("PPD"))` with original `perror` and return **-1** if null. The original does not implement the fallback described in its warning;
- `cupsParseOptions(argv[5],0,&options)`, then `ppdMarkDefaults(ppd)`, before iterating options in order;
- log **every** option at original line **115**, including those whose values are null; only invoke `ppdMarkOption` when the value is non-null; log nonzero conflicts at original line **123**;
- log `process options` at original line **128** and invoke `cupsMarkOptions` exactly once, ignoring its return value.

Unlike the Linux 1.0.3 manufacturer source, this original macOS loop has **no special handling for `ContinuousPaperLength`**. That Linux-only behavior has not been imported. The already reconstructed six choices in `original_raster_ppd_options` are the subsequent original region and were **not duplicated or modified**.

The analysis-only callback structure exposes CUPS handles and the still-owned options array so a future full `_raster2printer` can continue with the original next call. It is not the OEM native ABI and is **not** connected to the active CUPS adapter. The OEM's incomplete cleanup in early return branches, unbuffered stderr, null-option logging, ignored `cupsMarkOptions` result, and early return values have been retained rather than hardened.

Local GCC/Clang strict C11 builds and mock tests passed for normal fd/stdin routes, original callback order, conflicts and stderr, invalid argc, missing raster input, missing raster stream, and missing PPD. These tests exercise a reconstructed function with callback CUPS mocks, **not** the original Mach-O private function. CI will verify both Linux/macOS compilation and the pre-existing manufacturer comparisons; results must be checked before claiming success.

## 2026-10-14 — Continuous original raster options startup integrated

After confirming that `sources` already contains separately recovered
`original_raster_preflight` and `original_raster_ppd_options`, the original
raster2printer assembly was checked for the adjoining region
`0x1000052d1..0x10000548e`.

New `original_raster_option_initialization.[ch]` is an **isolated
integration** of these two previously recovered functions into the
manufacturer's original active startup sequence:

1. Zero the complete `0x10a8`-byte original descriptor and set
   `OptimizationLevel=1` **before** invoking the CUPS preflight.
2. Execute `setbuf`, `argc` validation, `cupsRasterOpen`, `ppdOpenFile`,
   `cupsParseOptions`, `ppdMarkDefaults`, per-option `ppdMarkOption`,
   conflict diagnostics and `cupsMarkOptions` in the already-recovered
   original order. An original early error exits immediately.
3. Select exactly the six macOS manufacturer PPD choices in the same
   original sequence, reusing `original_raster_ppd_options.c` without
   duplicating the algorithms.
4. Call `cupsFreeOptions`, then `ppdClose`; **only after these calls**
   write the manufacturer's line 249 option diagnostic.
5. Invoke `_OpenDebugFile` unconditionally, as the Mach-O does at
   `0x100005418`, even if the debug-mode option is off.
6. Set `__fd_stdout` to null when `PrintThroughPRN=ON`, or to stdout
   otherwise. Prepare an independent **byte-for-byte copy** of the
   original `0x10a8`-byte descriptor and call `_StartDocument`
   exactly once at `0x10000548e`.

The callback representation of the original CUPS handles,
`__fd_stdout` pointer and by-value descriptor argument is newly
authored analysis scaffolding, not the manufacturer's ABI. **The
actual CUPS adapter, original vendor files, halftone logic and all
existing successful reconstructed functions remain unchanged.**
In particular, the core printer-page loop after `_StartDocument`
has not been integrated and the new source is not a production
driver.

Local strict GCC and Clang tests passed, including full callback
ordering, descriptor copy identity, deferred PRN/stdout selection,
and early PPD failure which avoids all later calls. A separate test
covers the initial CUPS errors and conflict diagnostics. The
new integration remains to be compiled and checked by the
macOS/GCC/Clang CI run; original binary execution equivalence is
not yet claimed.

Previous preflight commit `ea9f9f103e15d3aa1e75d1f73bf1694311b47f79`
was verified by [run 38495904705](https://github.com/an3k/leitz-icon-linux/actions/runs/38495904705):
**all three jobs completed successfully** (GCC, Clang, native macOS and
the existing original-binary comparison suite). Those reference
tests do not exercise the newly connected startup section inside
the original Mach-O directly.

## Verified CUPS startup reconstruction builds — 2026-10-14

The new macOS classic CUPS preflight and early error/PPD-option
processing at commit `ea9f9f103e15d3aa1e75d1f73bf1694311b47f79`
was validated by [run 38495904705](https://github.com/an3k/leitz-icon-linux/actions/runs/38495904705),
**all three jobs successful**. This included GCC/Clang C tests, native
macOS build/tests, and the prior manufacturer-original comparison suite.

The subsequent integration at commit `99f6cc7c4b137e4b6f9c80c6eef4d539c4f5fbef`,
joining the recovered CUPS preflight, six original PPD choices, option
free/PPD close, diagnostic, debug-file opening, stdout/PRN route and
by-value descriptor transfer to `_StartDocument`, was validated by
[run 38496310468](https://github.com/an3k/leitz-icon-linux/actions/runs/38496310468):
**all three jobs successful**, including GCC, Clang, native macOS and
the existing original-vendor comparison suite. The newly joined call
order and memory copying have been tested under mocked CUPS callbacks;
these specific internal manufacturer branches have **not** been directly
invoked inside the native original executable.

No changes to `originals/`, `enhancements/`, the active Linux/Unraid
driver, or previously recovered C implementations were made. The original
native job/page-processing body after `_StartDocument`, its precise
ABI and complete hardware behavior still require reconstruction.

## 2026-10-14 — OEM active inline raster page acquisition recovered

After confirming branch `sources` remained at
`9c22be4bee0f50ac5a4803d3357741562292d211`,
the original macOS 5.1.465 `raster2printer` `_raster2printer` Mach-O
was examined directly from the SHA-256 verified manufacturer's binary
(`72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`),
at `0x1000056c3..0x100005f22`.

`original_raster_active_page_read.[ch]` reconstructs the **previously
missing inline page-acquisition branch of the active `_raster2printer`
routine**, not the existing separate exported `_ReadPage` (which remains
unchanged). The two OEM functions have materially different behavior.

Evidence-grounded details:
- `cupsRasterReadHeader2` is called once. A zero result prints
  `No more available page : %i` at original source line 346, retaining
  the previous requested-byte count.
- On a nonzero header it increments the page count and reports media type
  and size name at original line 300. It copies the original 16-bit page
  width/height, 32-bit CUPS stride, color space and bits-per-pixel into
  the already recovered original descriptor offsets, computes the original
  packed stride and uses the original unchecked `strcpy`.
- OEM diagnostic lines 310, 312 and 314 report media name, CUPS bytes per
  line and computed packed bytes per line.
- This **active** path calls `malloc` with the original signed 32-bit
  product of `cupsBytesPerLine * cupsHeight`. It reports original
  allocation success/failure at lines 322/326, then makes **one**
  `cupsRasterReadPixels` call, even if `malloc` returned null.
- Zero pixels resets the requested-byte count to zero and stops the page.
  It **does not free the page buffer here**. This differs from the
  previously recovered standalone `_ReadPage` which uses `calloc`
  and frees its buffer on a zero read.
- Any positive pixel count (including a short read) prints the original
  line 340 report and allocates a **second** `ditheredPage` buffer with
  `cupsBytesPerLine * (uint16_t)cupsHeight`, reporting original lines
  356/360. The allocation is **not** limited to a fixed paper length.

The test invokes a deliberately minimal CUPS fixture with a positive
short read, zero-pixel second page and EOF third call. It verifies exact
OEM diagnostic text, state transitions, page counter, truncated geometry,
original 24-bit packed-stride calculation, allocation ordering and
non-freeing of the zero-read buffer. The test frees only its own
buffers outside the reconstructed function.

**Scope limitations:** Existing callback-based CUPS adapter and
`_ReadPage` remain untouched. OEM reprint reuse, termination
after page output, diagnostic messages before this region, original
printing after the dither allocation, and allocation-failure undefined
paths are not recovered here. Test mocks do not call the private
`_raster2printer` function inside the manufacturer executable, and
no complete executable equivalence is claimed. No safety hardening,
fixed paper length, or altered manufacturer error behavior was added.

## 2026-10-14 — normal-page dispatch after active raster read

Disassembled the follow-on branch from the original SHA-256-verified
manufacturer's `raster2printer` Mach-O at `0x100005f3b..0x1000062e5`.
The new `original_raster_active_page_output.[ch]` consumes the
already reconstructed active inline page-reader state; no second page
reader, raster encoder or `_ProcessPage` implementation is introduced.

In the OEM normal, non-reprint page path:
1. If termination was requested or the "has page" flag is false,
   exit before the normal per-page output and cleanup fragment.
2. If the requested pixel count is positive and `pageBuffer` is non-null,
   call `_DitherPage` exactly when CUPS color space is **1** (RGB).
   Pass the original quality value at descriptor offset `0x4c`.
3. Call `_StartPage`, `_ProcessPage` with the zero-based page index
   and original fourth argument **0**, then `_EndPage`. RGB output
   passes the dithered buffer; non-RGB passes the original page buffer.
   OEM `_EndPage` receives a **full by-value copy of the original
   0x10a8-byte descriptor**, unlike `_StartPage` and `_ProcessPage`.
4. Emit the manufacturer's original line 393
   `shouldResumePrintingOfLastLabel = false` diagnostic.
5. For a missing/null page buffer, print `INFO: pageBuffer null` and
   set the process status to 1; skip page commands.
6. Increment the zero-based page index and print exactly
   `STATE: none` and `PAGE: N 1` on stderr. Free both dither and
   primary page buffers separately, setting both pointers null.
7. Continue reading pages **only if** `_ProcessPage` returned 0
   and the original "has page" flag was set.

The special OEM `shouldResumePrintingOfLastLabel` path is **not**
included in this new normal-page C function. In the analyzed main
function that local flag is initialized to zero and has no observed
normal assignment to one, but the original machine-code branch still
exists and its undefined/unreachable semantics have not been declared
equivalent. Full job-end `_EndDocument`, IPP/status decisions, signal
interactions and the original compiler ABI also remain outstanding.

The new tests verify RGB and non-RGB calls, existing quality selector,
exact start/process/end order, by-value descriptor copy, failure
propagation, missing buffer, termination/EOF skip, original CUPS page
status diagnostics and buffer ownership. The injected callbacks invoke
no real printer and do **not** substitute new printer protocol logic.
No existing reconstruction, original manufacturer file, PPD, or
`enhancements/` code has been overwritten.

**Verification:** New isolated GCC/Clang and native Apple CUPS SDK
compilation are configured via the existing workflow. The source
compiles/behavioral assertions and the pre-existing original-Mach-O
comparison must both succeed before the result is reported as verified.

## Verified native builds and OEM comparison for active raster page fragments

- [Run 38497080066](https://github.com/an3k/leitz-icon-linux/actions/runs/38497080066), commit `363f07b30f854d2762a05e198f1877f54cf2b38d`: **all three GitHub Actions jobs successful**. Isolated original inline raster page acquisition compiled and passed behavioral checks under GCC/Clang, native Apple macOS SDK compilation, and the existing original OEM execution comparisons. The original private inline page-read branch was not directly invoked with injected inputs.
- [Run 38497491545](https://github.com/an3k/leitz-icon-linux/actions/runs/38497491545), commit `379983dcadd7c74f08d4ca1aab8decaa418be901`: **all three jobs successful**. Isolated normal page dispatch, calls to the separately reconstructed OEM page functions, by-value `_EndPage` descriptor and original CUPS-status/buffer cleanup were checked under GCC/Clang, native macOS build, and the unchanged OEM reference suite.

Both new modules remain **source-level reconstruction components**. They do not fully connect the original `_raster2printer` startup, the active raster acquisition, normal page dispatch, reprint/cancellation, device status, or `_EndDocument` into a single manufacturer-equivalent executable. The existing OEM execution comparisons cover their established reference scenarios, not a direct side-by-side invocation of these newly reconstructed private-function branches.

## 2026-10-14 — OEM raster job epilogue recovered

After checking the existing `sources` files at `e95b269152e17f354b24ace2246a48a89904f625`,
the manufacturer's **actual** `raster2printer` x86-64 Mach-O was read locally
(confirmed SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`).
The previously missing active `_raster2printer` function exit at
`0x1000062eb..0x10000685a` was translated into
`original_raster_job_epilogue.[ch]` with C-level callbacks only for already
known OEM helper calls. No original vendor source or prior reconstruction
was overwritten. This segment follows the separately recovered page loop,
but the two functions are **not** wired into a complete OEM-compatible binary.

The assembly explicitly establishes the following original sequence:

1. Call `_EndDocument` with a **by-value 0x10a8-byte descriptor** copy,
   including when the job was aborted or the local return code is nonzero.
2. Close the raster input fd only if **fd != 0**; call `_CloseDebugFile`
   unconditionally.
3. If the original descriptor's 32-bit PRN option at offset `0x08` is
   nonzero, call `_SendDebugFileToPrinter` and print the original
   `raster2printer.c:440` PRN diagnostic; otherwise print line 444.
4. If `strstr(descriptor+0x50,"ipp")` is nonnull, the raw **8-byte slot
   at descriptor offset `0x1090` is nonzero**, and socket fd != -1,
   close the socket. The OEM's private meaning of that slot is not
   guessed; the original **raw condition** is reproduced.
5. If `terminate_flag` is nonzero, build `ESC @ ESC Z 00 00`,
   increment the original global `job_size` by **six**, and invoke
   `_WriteData` with all **six** bytes. In particular, do not
   silently shorten this OEM write to the four non-NUL command bytes.
6. With signed 32-bit `job_size % 64 == 0`, invoke `_WriteData`
   once more for a **single `ESC` byte**, including when
   `job_size==0`. OEM does **not** increment its counter for this extra byte.
7. Emit exactly `STATE: none`, the manufacturer job-size diagnostic
   `raster2printer.c:471`, and the finished diagnostic
   `raster2printer.c:472`, then return the original status result.
   Individual `close`, PRN transfer and `_WriteData` results are ignored.

Original x86-64 stack-passed **descriptor copies** are represented
through an analysis callback and in-memory by-value copies. This
reconstructs defined side-effect order, payload length and byte values
**but does not restore** the original native machine calling convention,
global addresses or full program ABI.

Tested locally with real GCC and Clang and Clang ASan/UBSan against
standalone C mocks for direct stdout vs deferred PRN mode, input and
socket close order, positive/nonzero and zero socket predicates,
aborted job, preserved return code, complete six-byte abort payload,
extra one-byte `ESC` at 64-byte boundaries (including zero),
the ignored OEM helper return values and **exact original stderr bytes**.
The existing OEM print-output comparison suite is separate. Native
macOS and original-comparison GitHub Actions results are recorded only
once the corresponding new commit has completed successfully.

**Still missing:** Real `_ProcessPage` branch integration, original
page-loop reprint/cancellation decisions, active device status and
transport state, and a single complete rebuildable raster2printer
executable. This new source is *not* linked into the production driver.

## Verified original job-exit recovery build — 2026-10-14

New independent `_raster2printer` job epilogue at commit
`f085dca1c245c384adb511dae64c6b118c1e99cd` passed
[Sources - Reconstruction Compile run 38498514631](https://github.com/an3k/leitz-icon-linux/actions/runs/38498514631):
**all three jobs successful** — native GCC and Clang tests, native macOS
compilation and tests with Clang, plus the established original
Leitz 5.1.465 Mach-O execution comparisons. The actual original
`_raster2printer` internal exit branch was analyzed directly from the
verified Mach-O bytes; the newly exposed callback routine was tested
against a mock environment. No claim of direct OEM-private-function
execution equivalence or whole-program identity is made.

**Newly evidenced:** The original job-end abort write transmits SIX
bytes `1b 40 1b 5a 00 00`, not merely four command bytes. Another
single `1b` byte is sent when original signed job-size modulo 64 is
zero, including for a zero-size job, without incrementing the counter.
These original behaviors should not be silently 'fixed' inside the
`sources` reconstruction. Native reprint/transport state and complete
OEM executable rebuild still remain uncompleted.

## 2026-10-14 — connected original raster startup-to-epilogue C path

Branch `sources` was first checked at commit
`0bd3178d8393332be3e1b26344b3c3d16d842224`.
The preceding standalone source fragments and reference tests were already
present and successful. No prior reconstruction is replaced or duplicated.

`original_raster_connected_job.[ch]` provides a **new, separately named,
callable C composition** of four already reconstructed original
`_raster2printer` regions:

1. `original_raster_option_initialization`: argument checks, CUPS
   raster input, PPD options, cleanup, output route and `_StartDocument`.
2. `original_raster_active_page_read`: manufacturer `malloc` and single
   CUPS pixel read; NOT the differently implemented standalone `_ReadPage`.
3. `original_raster_active_page_output`: normal non-reprint RGB/mono
   output, `_DitherPage`, `_StartPage`, `_ProcessPage`, `_EndPage`,
   status and page buffer release.
4. `original_raster_job_epilogue`: `_EndDocument`, input/PRN/socket
   closure, original six-byte abort sequence and 64-byte-boundary
   `ESC`, diagnostics, original return-slot value.

The newly authored connector passes the CUPS handle from the preflight
into the recovered `0x10a8`-byte descriptor, preserves zero-based
printed-page indices separately from CUPS pages read, and stops
on EOF/zero pixel count, a nonzero `_ProcessPage` result, or termination.
It passes the last process result, raw socket descriptor, global job-size
pointer, and cancellation state to the unmodified original job epilogue.

**Precisely limited evidence:** This is an integration of recovered OEM
fragments with a **new callback interface**. The exact OEM transitions
between these functions, native stack/global ABI, `shouldResumePrintingOfLastLabel`
(reprint), IPP/status device control, allocation-error branches, and
any undocumented fallback paths are **not independently proven**.
The integration is NOT a byte-identical decompilation of the whole
original executable and must not replace the active Unraid/Linux driver.
In particular, a zero-pixel read in the OEM inline reader leaves
`pageBuffer` allocated and the normal output branch is skipped.
This is exposed to the analysis caller without silently introducing
new OEM cleanup or changing original error behavior.

The new test builds **all seven previously recovered source modules
together**, with one CUPS fixture and callbacks standing for the still
unlinked original CUPS/transport functions. It verifies a complete
two-page mono/RGB job through `_EndDocument`, process-return failure,
invalid argument early exit, early termination with SIX OEM abort bytes
plus trailing `ESC` at 64-byte boundary, and zero pixel result
retaining buffer ownership for explicit test cleanup. The prior
original-bin comparison suite is preserved; no new native-OEM parity
claims are made from mock tests.

Native Clang/CUPS SDK compilation is added as a separate check. Real
printer communication, reprint paths, and exact whole-binary equality
remain open. The GitHub run must be checked before any CI success claim.

## Connected original raster C job verification — 2026-10-14

At `sources` commit `11029bfc7ec4eded160f61ad57eec75e72832133`,
[GitHub Actions run 38545249981](https://github.com/an3k/leitz-icon-linux/actions/runs/38545249981)
completed successfully in **all three jobs**. Original-derived
`original_raster_connected_job_test` compiled and ran successfully
under GCC and Clang on Linux, and in the native macOS build. The
existing SHA-256-verified manufacturer-original comparisons passed.

The test links all previously recovered `_raster2printer` startup,
active page read, normal page output, and job-end fragments together,
instead of merely mocking their control transitions independently.
Verified scenarios: two pages with mono and RGB raster processing,
ProcessPage nonzero exit, invalid argument early return, abort before
the first page with the six-byte OEM abort sequence and extra `ESC`,
and a zero-pixel read that leaves OEM page-buffer ownership unchanged.

**What this does not prove:** exact equivalence of the original macOS
`_raster2printer` function. The connector has a newly designed C
callback API and unverified OEM transition points, and does not yet
handle IPP/status, reprint, undocumented error cases or the original
full C++/Mach-O ABI. No hardware test occurred. The earlier, unrelated
normal CUPS candidate and all original sources remain untouched.

## 2026-10-14 — manufacturer-confirmed raster-loop diagnostics and page index

The full original manufacturer `raster2printer` Mach-O was audited
at the three previously unverified control transitions in
[run 38546191969](https://github.com/an3k/leitz-icon-linux/actions/runs/38546191969).
`analysis/macos-5.1.465-classic/RASTER_LOOP_NATIVE_AUDIT.md`
records the exact original instruction addresses and source format literals.

Two `started printing pages` diagnostics (source lines 256 and 258),
emitted directly **after** `_StartDocument`, were absent from the
newly written `original_raster_connected_job` glue. A third
diagnostic, `Not reprinted i_page = %i.`, is emitted **before**
each normal page-header read, including the iteration that reaches EOF,
and was also missing. Those three messages have been added to the
connecting reconstruction **without** altering its OEM-derived
individual functions or introducing a new printing protocol.

The connected test now verifies exactly four successful startup
sequences among its five scenarios, exactly five non-reprint
messages (3 normal two-page/EOF, 1 failed page, 1 zero-read),
the original page indexes 0/1/2, correct message ordering, and
no startup diagnostics for an invalid-argument early exit.
The non-reprint branch is the only path currently executed by the
connected C callback. The native reprint path is independently
visible in the original instructions but has NOT yet been proven
reachable or reconstructed in full.

An additional native audit of compiler-local reprint/has-page
flags has been added to the original vendor verification script;
verify its results before claiming any conclusion about reprint
reachability. There is no claim of whole-binary equivalence.

## 2026-10-14 — original raster flag/call inventory confirmed

After the corrected native audit in
[run 38547714269](https://github.com/an3k/leitz-icon-linux/actions/runs/38547714269)
(**all three jobs successful**), the following now have direct
manufacturer-machine-code evidence:

- The `-0x7c05(%rbp)` reprint flag is initialized to **0** exactly once
  (`0x100005603`), then only read at six instruction addresses, with no
  direct store of a nonzero value or direct address escape within the
  complete original `_raster2printer`. Therefore the ordinary
  defined control flow does **not activate** the compiled reprint path.
- `_raster2printer` makes **153** original direct machine-code calls,
  with no direct `_CheckPrinterStatus` or `_ResolveDeviceURI` call.
  Consequently status/URI logic must not be invented inside the active
  page loop; potential use by other manufacturer functions remains
  to be inspected.

Exact instructions are tabulated in
`analysis/macos-5.1.465-classic/RASTER_LOOP_NATIVE_AUDIT.md`.
The connected C reconstitution continues to execute the normal
no-reprint branch. Its two OEM start diagnostics and per-iteration
page-index log have separately passed Linux GCC/Clang and native
macOS tests and the existing manufacturer comparison suite.
These results do **not** prove whole-program or hardware equality.

## 2026-10-14 — Full native StartDocument and EndDocument side effects

Before changing source files, branch `sources` was confirmed unchanged
after commit `cfd70273c5beeb21e3cbf4e2d8c528138f36b7d4`.
The original Leitz 5.1.465 macOS raster2printer executable
(SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`)
was disassembled at both original function bodies in
[run 38549177080](https://github.com/an3k/leitz-icon-linux/actions/runs/38549177080).
**All three jobs succeeded**. The `_StartDocument` region
`0x100006890..0x100006dbb` has 327 OEM machine instructions, including
121 explicit ESC-byte stores, and `_EndDocument`
`0x100008ac0..0x100008cdc` has 105 instructions.

**New original function recovery:**
`original_raster_document_lifecycle.[ch]` models the full **defined**
native output/state sequence, reusing the previously tested command
builders from `raster2printer_boundaries_recovered.c` **without
duplicating or overwriting them**.

- `_StartDocument` unconditionally sends the original 122-byte
  prefix (121 ESC followed by `@`), incrementing `job_size` by 122.
  Only when descriptor `+0x14` indicates high resolution does it
  make a **second, separate** `_WriteData` call for `ESC i` and
  increment `job_size` by 2. No flush is performed here. Two sends
  rather than one combined 124-byte write are a native behavior that
  the earlier byte-only model could not demonstrate.
- `_EndDocument` checks original cutter field at `+0x10`
  and global termination flag. **Only if both are zero**, it sends
  `ESC E`, increments `job_size` by 2 and immediately calls
  `_FlushDataStreams`. It always writes `ESC @ ESC Z` as a
  **separate four-byte** command and increments `job_size` by 4,
  with **no second flush**.
- Immediately after the final command, it unconditionally invokes
  `cupsRasterClose(descriptor->cupsRaster)` using the pointer at
  original descriptor `+0x00` and sets that pointer to NULL. It
  does **not** close the input file descriptor here; that belongs
  to the surrounding `_raster2printer` epilogue.
- All `_WriteData` results are ignored and the manufacturer function
  is `void`. Each write receives its own independent 0x10a8-byte
  by-value descriptor copy and the original socket descriptor.

A standalone C test checks normal 300dpi, high-res 600dpi,
cutter-on, terminated, and NULL CUPS-pointer branches, **separate**
write calls, call order, precise payloads, job_size increments,
ignored negative send results, conditional flush, CUPS close and
descriptor pointer reset. No arbitrary paper-length limit is used.

The test ABI uses injected `_WriteData`, `_FlushDataStreams`
and `cupsRasterClose` callbacks because the complete OEM global /
C++-linker ABI is not yet recovered. The original binaries themselves
were disassembled and matched to source behavior; direct invocation
of their private functions with injected side effects remains
unverified. The new source is not wired into a live CUPS filter,
and no real printer has been used. Recovered active driver output
and manufacturer original files remain unchanged.

## Verified original document boundary source build — 2026-10-14

Commit `1232cc453b5a5cd6659923eba668cfe86bce68fb`
passed [GitHub Actions 38549748927](https://github.com/an3k/leitz-icon-linux/actions/runs/38549748927):
**all three jobs successful** (Linux GCC and Clang, native Apple
macOS/Clang CUPS SDK, and the unchanged SHA-256-verified Leitz 5.1.465
original-executable reference comparisons). The new standalone C
document lifecycle test actually compiled and executed on GCC, Clang,
and macOS, proving the defined original _StartDocument/_EndDocument
write/counter/flush/CUPS-close branches.

Scope: Original OEM function bodies were disassembled directly in the
preceding successful [run 38549177080](https://github.com/an3k/leitz-icon-linux/actions/runs/38549177080)
before creating the implementation. Native x86-64 stack/global ABI
reconstruction and direct calling into original Mach-O functions with
controlled CUPS/write mocks were NOT performed. Original Linux source,
macOS manufacturer files and the active Unraid driver are unchanged.
The newly recovered functions still have to be bound to the full
native executable.

## 2026-10-14 — original document lifecycle connected to complete raster C job

The actual branch `sources` was checked at
`6f340d5888b9e400341a35bba7503e9f98573161` before making any changes.
The recovered `_StartDocument` and `_EndDocument` functions, native
manufacturer disassemblies, byte encoders, previously integrated
`_raster2printer` job and successful CI tests were already present:
**none has been reimplemented**.

A real control/data-flow mismatch was found in the previously joined
analysis source: `original_raster_option_initialization.c` supplied
the original OEM `0x10a8` descriptor to `_StartDocument` **before**
copying the `cupsRasterOpen` result to its offset `+0x00`.
The genuine OEM stores the handle in that descriptor during the CUPS
preflight, before `_StartDocument`. The pointer is now copied
immediately after the preflight and retained on a PPD-open failure,
as in the original. The later redundant descriptor assignment in
`original_raster_connected_job.c` was removed. Existing tests were
updated to assert the correct handle at `_StartDocument`, rather
than re-running or replacing completed stages.

New `original_raster_document_bridge.[ch]` binds the **already recovered**
`original_raster_document_lifecycle.c` functions directly to the
existing callbacks at the actual `_StartDocument` and
`_EndDocument` invocation points. The manufacturer passes each
`0x10a8`-byte descriptor by value. Thus `_EndDocument`
calls `cupsRasterClose` and zeros the pointer in its **local copy**;
it does not reset the original caller's descriptor. The bridge
deliberately preserves that difference, rather than modifying
the manufacturer's memory ownership model.

New `original_raster_document_bridge_test.c` links all existing
source-level modules together, not just isolated stand-ins, and tests:

- a complete one-page CUPS print with the exact 122-byte
  `_StartDocument` write, conditional 2-byte `ESC E`, flush,
  4-byte `ESC @ ESC Z`, raster close and the OEM one-byte trailing
  `ESC` on a 128-byte job-size boundary;
- 600dpi `ESC i` on a **separate** second start write and
  cutter-on `_EndDocument` without a flush;
- early SIGTERM, the genuine six-byte reset/end abort write and
  no raster page read;
- PPD opening failure and invalid argc with no document start/end;
- the original CUPS stream handle present during startup,
  a single close at job end, and the unchanged by-value caller
  descriptor after `_EndDocument`.

The new bridge retains the callback interface of the existing
source-level reconstruction, **not** the original C++ Mach-O ABI.
CUPS calls, page printing and `_WriteData` are injected locally.
These tests do not prove native whole-executable equivalence or real
device operation. The active Linux/Unraid driver, vendor original
sources and all original command-byte encoders remain unchanged.

## 2026-10-14 — connected OEM document lifecycle CI verified

The implementation at
`f2074dad8ac23b27bd905bd9745c3b43c47efe5e` passed
[Sources - Reconstruction Compile run 38550981146](https://github.com/an3k/leitz-icon-linux/actions/runs/38550981146):
**all three jobs completed successfully**. Linux GCC and Clang both
compiled and executed `original_raster_document_bridge_test`;
the native macOS Clang job compiled and ran the combined test and
the new bridge against the actual Apple CUPS SDK. The unchanged
manufacturer Mach-O execution comparisons also passed.

The combined test confirms the OEM raster descriptor's opened CUPS
handle is available before `_StartDocument`; connected one-page
print/EOF; separate 122-byte start, optional 2-byte 600dpi
command, 2-byte final `ESC E`, conditional flush and 4-byte
document end; one CUPS stream close; by-value descriptor ownership;
signed job counter / extra `ESC`; early termination and missing
PPD/invalid argc behavior. These are source-level reconstruction
and injected callback tests, not direct execution of the
original private document functions under mock inputs.

Remaining: Direct OEM ABI reconstruction and integration of authentic
`_ProcessPage` and original device/IPP transport, error and status
branches. No Leitz Icon hardware test or change to the active Linux
driver occurred.

## 2026-10-14 — Full native StartPage/EndPage side effects recovered

Branch `sources` was verified before modifications, including the prior
`_StartDocument`/`_EndDocument` integration and successful CI
[38550981146](https://github.com/an3k/leitz-icon-linux/actions/runs/38550981146).
The original manufacturer's SHA-256-verified 5.1.465 `raster2printer`
Mach-O was then examined under native macOS:
[run 38552792069](https://github.com/an3k/leitz-icon-linux/actions/runs/38552792069),
**all three jobs successful**. The new audit captures all 100 instructions of
`_StartPage` at `0x100007380..0x10000755d` and all 55 instructions
of `_EndPage` at `0x1000089b0..0x100008ab9`.

New `original_raster_page_lifecycle.[ch]` translates the **defined full
manufacturer page boundary call and state sequence**, and reuses existing
original command-byte builders rather than implementing them again:

- `_StartPage` reads width/height as **16-bit** fields at descriptor
  `+0x1e`/`+0x1c`, high resolution at `+0x14`, emits exactly
  **one 11-byte** `_WriteData` call with `ESC L`, `ESC w`,
  `ESC B 00` and increments the original global job counter by 11,
  independent of the write return value.
- Verified **original 16-bit arithmetic**: add 30 rows (low resolution)
  or 60 rows (high resolution), *truncate this sum to 16 bits*, then
  divide by two only for high resolution. The existing byte-builder
  formerly computed the sum with wider arithmetic and could disagree
  for heights near `65535`. Only that evidenced calculation has been
  corrected. Original paper widths and variable lengths are retained.
- `_EndPage` reads the original cutter field at `+0x10` and sends
  **one** `_WriteData` of two bytes `ESC G` if cutting is off, or
  four bytes `ESC G ESC E` if on. It increments the job counter by the
  full command length, ignores the write's return value and calls
  `_FlushDataStreams` **unconditionally after writing**, regardless
  of cutter option.
- Every `_WriteData` receives the manufacturer's full independent
  `0x10a8`-byte descriptor copy and original socket descriptor.
  `_StartPage` receives a descriptor pointer, whereas `_EndPage`
  receives its own by-value descriptor at the manufacturer call site.

A new test checks manufacturer payload bytes, counter increments,
16-bit height wraparound at `65535`, high/low resolution, cutter
on/off, the ignored negative write result, the by-value copy and the
unconditional page flush. The existing document bridge integration
test now invokes these native-derived page functions rather than
only simulated page start/end callbacks, verifying their combined
effects with the previously recovered document lifecycle.

These remain source-level OEM reconstructions with injected callback
transport and a new C ABI; full Mach-O process-global, C++ calling
convention, `_ProcessPage` and device transport integration are NOT
established. The active Unraid driver and manufacturer original
files remain untouched.

## OEM native page functions integrated and CI-verified — 2026-10-14

At commit `ae84c68a09640d8049958c40c3a9afd2a5b0c04c`,
[Sources - Reconstruction Compile run 38553514817](https://github.com/an3k/leitz-icon-linux/actions/runs/38553514817)
completed with **all three GitHub Actions jobs successful**. This was
verified from the per-job conclusions and actual compiler logs:

- Linux GCC and Clang compiled and executed the independent
  `original_raster_page_lifecycle_test`, checking native OEM command
  bytes, per-command job-size increments, unconditional end-page flush,
  separate by-value descriptor, negative WriteData result ignored,
  cutter, both resolutions and 16-bit overflow arithmetic at height
  `65535`.
- The previously complete document/startup/page-loop/epilogue test now
  executes **actual recovered StartPage and EndPage routines**, rather
  than just recording their calls, and validates resulting combined
  counters and payloads. Both GCC and Clang passed.
- Native macOS Clang built these sources against the real Apple CUPS
  headers and executed both tests successfully. The established
  SHA-256-verified original manufacturer execution comparison suite
  also completed successfully, with no regressions.

The isolated lifecycle implementations are derived from complete
original `_StartPage` and `_EndPage` machine instruction coverage
in [run 38552792069](https://github.com/an3k/leitz-icon-linux/actions/runs/38552792069).
A complete original private-function invocation under injected inputs
or real Leitz hardware has **not** occurred. `_ProcessPage` remains
the primary still-incomplete native-function reconstruction.

## 2026-10-14 — ProcessPage OEM modes 0/1 control-loop recovery

Before modifying files, `sources` was confirmed at
`e2e568155b1fa7b1d51fd5d9ed7681e5e25de0d3`; the already
reconstructed four isolated row encoders, macOS CUPS adapter and
document/page lifecycle were present and CI-verified. Neither the
original manufacturer binaries nor those completed encoders were
rewritten.

A new native macOS original-Mach-O audit at commit
`bd0e1d5dd5f58226b940bf8366ffff1813091c51`
captured the complete 165-instruction original `_ProcessPage`
entry/setup/progress segment `0x100007560..0x1000078eb` and
the original 68-instruction output/termination/return segment
`0x100008850..0x1000089b0`. It was run against the
SHA-256-verified OEM file:
`72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`.
These are explicit native machine-code fragments, not inferred
from the different Linux OEM version.

New `original_raster_process_page_modes01.[ch]` creates a **callable
normal-case ProcessPage C control loop for manufacturer optimization
modes 0 and 1**. It reuses the **unchanged**
`classic_process_page_modes_01.c` row encoder, but reconstructs
additional originally missing behavior:

- Per-page initialization and single four-byte `ESC D <stride> SYN`
  `_WriteData`, with the original global `job_size += 4`.
- 16-bit OEM CUPS raster height and 32-bit stride from the original
  `0x10a8`-byte descriptor; no fixed print length.
- At row indices divisible by 32, original
  `INFO: Printing page %d, %d%% complete...` and
  `ATTR: job-media-progress=%d` diagnostics (not a device
  acknowledgement), calculated using the original floating-point
  conversion and truncation.
- Mode 0: full row transfer of **packed stride + four bytes** per
  original `_WriteData`, with global job counter increments.
  The last row's four extra bytes may come from **uninitialized OEM
  memory** on a buffer sized to the pixel data alone. The new
  reconstruction explicitly requires caller-readable padding for
  deterministic tests; it does **not** silently assert those four
  bytes are genuine raster data or change the OEM encoder.
- Mode 1: defer all-zero rows using the original 16-bit skipped-row
  counter. When a nonwhite row appears, emit `ESC f` in a
  **separate** four-byte `_WriteData`, then issue the original
  packed-stride-plus-four row write. Trailing deferred whites
  remain unsent.
- Check the manufacturer's termination flag **after every
  processed row**, including the last or an empty row. The OEM's
  initial result slot is zero and normal cancellation returns zero.
  Original negative/zero `_WriteData` responses do not cause
  new recovery/error branches.
- Modes 2 and 3 are **not** implemented by this new entry point;
  `-2` is a documented reconstruction-only unsupported-mode
  sentinel. Existing isolated mode 2/3 algorithms are left
  untouched pending precise native write-call boundary/ABI proof.

The dedicated test covers all row-command write sizes and bytes,
separate skip-vs-row calls, 64-row progress diagnostics, 16-bit
trailing whites, per-row abort, stdout diagnostics, combined
job counter and ignored write errors, with an explicitly padded
test input for the original overread. It is a source-level callback
test, **not** an execution of original Mach-O private `_ProcessPage`
under controlled inputs. The native C++/stack/global ABI and full
mode 2/3 original control flow remain open. No live printer or
production Linux driver changes occurred.

## Original ProcessPage modes 0/1 control loop: native CI verified

Source commit `5cef248e9f364c8faf3a0bf9522da03b6b6dc0ba`
passed [Sources - Reconstruction Compile run 38555305488](https://github.com/an3k/leitz-icon-linux/actions/runs/38555305488):
**all three jobs successful**, confirmed from the actual GitHub job results.

The new `original_raster_process_page_modes01_test` compiled and
executed successfully with Linux GCC, Linux Clang, and native macOS
Clang. Cases included normal mode-0 rows, deferred two-white-row
mode-1 `ESC f` and separate normal-row write, original trailing
blank-row omission, 64-row CUPS progress output, termination after
the first row, the complete independent descriptor argument
and original job-size counter increments. The manufacturer's
5.1.465 executable SHA-256/reference test suite also succeeded,
but this suite did **not** directly execute its private
`_ProcessPage` function with matching injected callbacks.

New OEM-derived normal-case mode-0/1 loop is therefore **verified
as a standalone C reconstruction**, not as a completed native
`_ProcessPage` or full-original functional-equivalence proof.
Original modes 2/3 have existing isolated row encoders but their
complete write-call boundaries and state transitions still require
integration, as does live device transport. No production
Linux driver or manufacturer file was changed.

## 2026-10-14 — ProcessPage modes 2/3 native write-boundary control

After verifying branch `sources` at
`439605e7cc6a2419abc3581794bea6f6f76acef3`, the existing
original mode-2/3 row encoders, source inventory and previous
successful tests were inspected. No manufacturer file or previously
recovered encoder was replaced.

The original macOS 5.1.465 `raster2printer` Mach-O
(SHA256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`)
was disassembled at all direct mode-2/3 output calls by
[run 38555919428](https://github.com/an3k/leitz-icon-linux/actions/runs/38555919428),
**all three CI jobs successful**. Its original machine-code call sites
confirm that the existing combined row bytes need **multiple**
`_WriteData` invocations with separate job-size accounting:

- Both modes: deferred `ESC f` is emitted separately when a later
  nonempty row follows blank rows.
- Mode 3: `ESC B` at `0x100007c38`, `ESC D` at
  `0x100007d6d` as individually conditioned three-byte writes;
  the raw `0x16` selector and actual raster bytes are sent **together**
  at `0x100007e5e`.
- Mode 2: `ESC B` at `0x10000808c`, `ESC D` at
  `0x10000820e`, the **single-byte** raw/compression selector
  at `0x10000866e` or `0x1000087b6`, and the following
  separately transferred raw/run-encoded payload at
  `0x1000086fd` or `0x10000885d`.
- Both modes: the original status strings
  `INFO: Printing page` and `ATTR: job-media-progress` are
  emitted every 32 rows; the original termination flag is checked
  after each row. The original does not flush trailing deferred
  blank rows as a new command.

New `original_raster_process_page_modes23.[ch]` translates these
**normal-case** control effects and reuses
`classic_process_page_modes_23.c` without duplicating its bit-run
codec, trimming conditions, persistent width state, or row bytes.
It maintains the original per-call job-size count and passes an
independent `0x10a8`-byte descriptor copy to each output callback.
The per-job width cache is provided by the caller to retain values
across printed pages. It explicitly refuses unsupported non-2/3
modes via a **reconstruction-only -2** sentinel.

The standalone test validates exact combined OEM output bytes **and**
their distinct transfer boundaries for mode-2 compressed and raw
rows, mode-3 trimmed and untrimmed rows, persisted widths across
pages, reset per-page offsets, blank-row deferral, progress, per-row
SIGTERM and ignored WriteData failures. This is still a callback-based
C reconstruction and not proof of ABI-identical manufacturer
execution; out-of-bounds native data and all undocumented errors
remain outside the defined normal-case model.

The next stage is to connect modes 0/1 and 2/3 dispatch to the
already joined raster page coordinator without changing original
row encoders or adding a fixed media length.

## 2026-10-14 — Connect actual reconstructed ProcessPage modes to raster job

Following the successful [standalone mode-2/3 reconstruction run
38556728807](https://github.com/an3k/leitz-icon-linux/actions/runs/38556728807),
the existing connected `original_raster_document_bridge_test`
was inspected and extended, **not replaced**. It previously used a
mock `_ProcessPage` even though the original document, page-start
and page-end functions had already been joined and tested.

New `original_raster_process_page_adapter.[ch]` is a small
source-level callback dispatcher that reuses the **existing**
`original_raster_process_page_modes01` and
`original_raster_process_page_modes23` entry points. It owns a
per-job width cache for modes 2/3, selects the original PPD option
at descriptor offset `0x40`, passes through the original page
index and cancellation state and refuses unreachable nonzero
reprint and unknown optimization selections with a documented
**reconstruction-only -2** status. No row compression logic,
bit-ordering, halftone or OEM command bytes were duplicated.

The joined CUPS document-bridge integration test now additionally
runs a complete normal one-page `OptimizationLevel=2` and
`OptimizationLevel=3` job through the actual recovered
`_StartDocument`, `_StartPage`, mode-specific `_ProcessPage`
loop, `_EndPage`, `_EndDocument`, and final job epilogue.
The former standalone mock-ProcessPage tests for existing
PPD, 600-dpi, cutter, cancellation and early errors remain
unchanged. Each additional scenario checks exact resulting
command byte stream, independent `_WriteData` call sizes,
global job size, separate flushes and a single CUPS raster
stream close.

Important outstanding original behavior: Modes 0/1 read
four bytes after each raw scanline, including after the last
scanline. A caller supplying only the original exact-sized
page pixel buffer invokes an OEM **undefined overread**.
The new adapter preserves the original encoder and documents
the required readable padding instead of silently replacing
undefined original memory bytes with invented data. Only
modes 2/3 are executed via this adapter from the joined
one-page raster test in this change.

The above is an analytical C ABI and CUPS fixture, not yet a
working, whole-original macOS executable or an actual Leitz
hardware run. The original vendor sources and the live Linux
driver have not been modified.

## Native ProcessPage modes 2/3 and joined printer path: final CI proof

- Standalone source reconstruction commit
  `1c93e65ef448ef36a6ae74a2db6e60a8ba47e925` passed
  [run 38556728807](https://github.com/an3k/leitz-icon-linux/actions/runs/38556728807):
  all three jobs successful. GCC, Clang and native macOS compiled and
  ran the new modes-2/3 control test. Original manufacturer SHA-256
  checks and existing binary fixture comparisons also passed.
- Joined normal-page C job commit
  `7dbae212115bbb4940e3335c83f35fc251ab067d` passed
  [run 38557369026](https://github.com/an3k/leitz-icon-linux/actions/runs/38557369026):
  all three jobs successful. The pre-existing whole-job fixture now
  executes genuine recovered mode-2 and mode-3 ProcessPage loops
  (not mocked ProcessPage) between the previously recovered page and
  document boundaries. The actual compiler logs confirm successful
  execution on both Linux compilers, native macOS and the unchanged
  manufacturer original comparison suite.

**Precise status:** Defined normal-case source-level command and
call boundaries for all four ProcessPage optimization modes now have
separate standalone C implementations, and modes 2/3 are also wired
into the joined multi-function job. The native executable's C++ ABI,
OEM out-of-bounds four-byte tail behavior in modes 0/1, exact mode-2/3
unknown-state/error behavior, and real device transport are NOT
yet completely reconstructed or proved equivalent. Existing 5.1.465
OEM reference fixture comparisons do not invoke the private native
ProcessPage with matching injected callbacks; they are regression
checks, not a complete parity claim.


## 2026-10-14 — recovered OEM RGB24 DitherPage connected to the normal raster job

Branch `sources` was checked at `4cb0cc1adcf2b030abc91ce001339ae1ce408a11`
before modifying any code. The actual `_DitherPage` reconstruction,
manufacturer halftone tables, four isolated `_ProcessPage` modes and
connected normal-case mode-2/3 job were already present. None was
reimplemented, modified, or replaced.

`original_raster_dither_bridge.[ch]` now connects the existing
`classic_original_dither_page` to the existing active-page
`dither_page` callback. The connected raster job fixture additionally
executes a complete **8-pixel, 24-byte RGB24 black-row** raster input,
using the OEM `Bayer8x8` choice and recovered halftone matrix
selection. Its eight generated monochrome bits are passed into the
real reconstructed mode-2 `_ProcessPage` (not a mock), followed by
the existing recovered `_EndPage`, `_EndDocument` and job
epilogue. Tests check the exact output call partition, 149-byte
combined stream, page/document flush order and raster close.
An obsolete test-only quality selector was removed to link the
existing OEM-derived selector instead.

**Verified**: [Sources - Reconstruction Compile run 38559047681](https://github.com/an3k/leitz-icon-linux/actions/runs/38559047681)
completed successfully in all **three jobs**: Linux GCC and Clang,
native Intel macOS Clang, and the independent authenticated original
Mach-O comparison suite. The vendor comparisons are separate
regression tests and do **not** directly invoke OEM private
`_DitherPage` or `_ProcessPage` with the test's injected callbacks.
The bridge establishes source-level C integration for defined RGB24
input, not the entire manufacturer native ABI, arbitrary pixel
geometry, undefined cases, or hardware equivalence. It has not been
installed as a printer filter; the active Linux driver is untouched.

Commits: `a1665fbe018452676ff4045aaf861c13a9a23a31` (integration)
and `75f30a367e3dd3390247f46df64338c8cccebc36` (remove obsolete
duplicate test selector after CI identified symbol collision).

## 2026-10-14 — original CUPS candidate verified against native OEM executable

Actual branch `sources` was inspected at
`920bb13a5c6ac531f98b2b05579f0ac271b54e27` before continuing.
The existing document, page, raster, dither, error handling and
status reconstructions were not replaced.

**Status path** (`769ba82aa6590bcc75b494112fb9527ff9b21cc1`):
Direct disassembly of SHA-256-verified original macOS `_CheckPrinterStatus`
at `0x1000092a0..0x1000093e2` and `0x1000096d2..0x100009797`
established the `send(...,2,0)` / `recv(...,64,0)` arguments,
independent full **0x10a8-byte** OEM descriptor copy passed to
`_WriteData` with the original socket argument, and the ignored
write return. The existing isolated C callback interface and test
were corrected. The original reset is still four bytes
`ESC @ ESC Z`, and `job_size` increases by four. The original
undefined socket-short-send / unchecked buffer conditions remain
outside the modeled paths. This function remains isolated rather
than being arbitrarily injected into a printer loop in which no
direct OEM call to it exists.

**Joined executable** (`0e27059cda167c4e5b66d053245c09c26d609328`,
with macOS-only build correction
`f218e1e6e9680911cde173076b2ebdfc96a412ca`):
`original_raster_cups_candidate.c` binds the already reconstructed
manufacturer-derived `_main`, CUPS/PPD preflight, document
lifecycle, RGB24 `_DitherPage`, `_StartPage`, mode-dependent
`_ProcessPage`, `_EndPage`, `_EndDocument`, output sink
selection, PRN debug helpers, cancellation and epilogue to real
Apple CUPS functions. Its executable
`original_raster2printer_cups_candidate` is separate from the
previously existing `classic_raster_cups_adapter`. It is an
original-derived **source-level ABI bridge**, not a binary-ABI-identical
decompilation and not a production install target.

**Direct OEM execution evidence**:
[Sources - Reconstruction Compile run 38561245944](https://github.com/an3k/leitz-icon-linux/actions/runs/38561245944)
successfully completed Linux GCC, Linux Clang, native macOS Clang
and manufacturer comparison jobs. The original `raster2printer`
Mach-O checksum was authenticated before execution. For the same
two-page CUPS inputs and PPD options, **both program return status
and all stdout bytes were identical**:

| Input | OptimizationLevel | Exit code | Byte-exact stdout |
|---|---:|---:|---:|
| 1-bit monochrome, 80 rows | 2 | 0 | 3,620 bytes |
| 1-bit monochrome, 80 rows | 3 | 0 | 7,523 bytes |
| RGB24, 80 rows | 2 | 0 | 14,889 bytes |
| RGB24, 80 rows | 3 | 0 | 14,410 bytes |

These are manufacturer **whole-executable** comparisons for four
specific inputs, not only isolated C callback tests. The original's
all-input, error/undefined, socket/IPP, file-logging and global
process ABI are still NOT completely recovered. Modes 0/1 have
original undefined final-row extra-byte reads and are intentionally
excluded from the new exact-match proof. No physical printer or
cut hardware has been used with the candidate. The independent
production Linux driver and original vendor source directories
remain unchanged.

## 2026-10-14 — 13 verified original command status paths

The `sources` HEAD was inspected at
`20719e60b7e6eb5e194d62867e4cf5b89fe7f82a` before beginning.
Previous OEM raster whole-job output matching, command main normal
entry tests, and the four original-command helper files already existed
and were not rewritten.

**Original-derived command C updates**:
- `command2leitz_recovered.c`: restored the original manufacturer
  `_GetPrinterStatus`, `_GetUnitedStatus` and `_GetFirmvareVersion`
  status-request diagnostics with original spelling and lines 123, 132,
  141; restored `_GetStatus` zero reply at original line 172 and
  read-error diagnostics including the original `errno` value at line
  166; corrected their exact newlines.
- `original_command_main_cups_candidate.c`: corrected the
  original `_GetStatus` CUPS `STATE:` line breaks for ready,
  paper empty, paper jam, general error, and unknown state; restored
  the source-line 208 permission/error report when
  `/private/var/log/leitz.log` cannot be opened and the line-217
  state-message diagnostic. Manufacturer-created side-effect paths
  have not been replaced with new retry or permission workarounds.
- `compare_connected_command_with_vendor.sh`: compares distinct,
  independently launched original Mach-O `command2leitz`
  and connected source-derived command entry using the existing
  synthetic macOS `cupsBackChannelRead` interposer. It requires
  **identical stdout bytes, identical complete stderr bytes,
  identical exit code and confirmed interposition**. Nothing is
  sent to a physical printer.

**Original binary verification and proof**:
[GitHub Actions 38564165909](https://github.com/an3k/leitz-icon-linux/actions/runs/38564165909)
verified the vendor command executable SHA-256
`6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee`,
then confirmed **13/13 exact full-program command entry parity cases**:
- `ReportStatus`: READY, NONE, MEDIA_JAM, READ_ERROR,
  MEDIA_EMPTY, GENERAL_ERROR, UNKNOWN_STATUS (OEM exit 1).
- `GetUnitedStatus`: NONE, U_42, READ_ERROR (OEM exit 0).
- `GetFirmwareVersion`: NONE, READ_ERROR, V_42 (OEM exit 0).

The native macOS CUPS build, Linux GCC/Clang reconstructed-code unit
suites, and existing original raster comparison suite also passed.
The 13 comparisons concern specific synthetic CUPS query responses,
not every possible status response, a live print backend, the
native Mach-O C++ ABI or a physical device. Original file,
network and invalid-memory error paths outside these defined cases
remain incomplete. The independent Linux driver, CUPS branch,
original vendor inputs and enhancements were not changed.

Implementation commits:
`4e078938fb5cc4b7c584e880e32629e9d47bbefd`,
`6d53e6a8f8e1c712f02b00e34bcb8aebe26f1299`,
`ab16e16c658b171ea903ec72589aba1d67cdf219`, and
`1d153a2bdde352eb316d9b3912cb2993c2d46216`.

## 2026-10-14 — original command utility functions connected to native global state

The live `sources` branch was verified at
`c7647361315876c0ae0fdbb4ed2de64275a5e46b`
before changes. The original command recovery already had all
four **isolated** helper semantics and 13 original command-query
comparisons. None of that existing implementation was replaced.

The SHA-256-authenticated original macOS command2leitz Mach-O assembly
in [run 38564165909](https://github.com/an3k/leitz-icon-linux/actions/runs/38564165909)
showed the precise four function bodies:
- `_FlushDataStreams` (0x100001c20..0x100001c5c):
  unconditional `fflush(stdout)`, followed by an optional
  `fflush(__fd_dump)`.
- `_is_bit_set` (0x100001c60..0x100001c86):
  32-bit shift masking as native x86 `shll %cl`.
- `_CloseDebugFile` (0x100002440..0x100002475):
  conditional `fclose(__fd_dump)` and clearing its global.
- `_CloseLogFile` (0x100002480..0x1000024b5):
  conditional `fclose(__fd_log)` and clearing its global.

These OEM function calls now use the **existing** recovered
`classic_command_original_utils.[ch]` implementations as their
underlying bodies, connected to real `__fd_dump`/`__fd_log`
symbols in `original_command_file_helpers.[ch]`. The already
reconstructed `_GetStatus` command CUPS entry now invokes the
connected original `FlushDataStreams` surface instead of the
isolated direct stdout flush callback. The candidate build links
the exact shared helper source files. The original global file
handling and x86 shift count wrap-around are unit-tested. No new
file-opening or device operation was introduced.

**Verified:** [CI run 38572871936](https://github.com/an3k/leitz-icon-linux/actions/runs/38572871936)
passed the Linux GCC/Clang compilation and tests, native macOS
compilation, and independent original manufacturer comparisons.
All 13 original whole-command (stdout + stderr + exit code)
comparisons and all four full raster-job mode-2/3 checks retained
their byte-exact parity. The close and bit helpers are directly
covered by isolated tests and disassembly, not dynamically
exercised by these 13 normal command scenarios.

Code commit:
`a1c33f3686fd4e24c15982db7f9cd17edaa4d210`.
The original C++ executable ABI, untested failure paths, and
device-side behavior remain beyond this increment.

## 2026-10-14 — connected OEM variable-length, PRN and FlushDataStreams

The current `sources` branch was checked at `81eb0a6422bd0d41cba2b5920e9bfdc2f7d42aec` before changes.
The existing native CUPS raster reconstruction, original PRN helpers and normal-case encoder paths were already present and were not replaced.

**Previously unverified complete output paths:** The separate `compare_connected_extended_with_vendor.sh` executes both the SHA-256-verified manufacturer Mach-O and the reconstructed joined macOS CUPS filter with identical two-page raster data. [Run 38574138676](https://github.com/an3k/leitz-icon-linux/actions/runs/38574138676) verified:
- 33 raster rows, `LabelCutter=OFF`, optimization 2: 1,616 identical bytes.
- 33 raster rows, cutter OFF, optimization 3: 3,313 identical bytes.
- 127 raster rows, cutter OFF, optimization 2: 5,738 identical bytes.
- 127 raster rows, cutter OFF, optimization 3: 11,855 identical bytes.
- `PrintThroughPRN=ON`, cutter ON, optimization 2: 3,620 identical stdout bytes **and** 3,620 identical PRN-file bytes.

These heights are CUPS **raster rows**, not physical millimeters. No fixed media length or printer hardware performance is inferred.
The manufacturer PRN directory is used only in disposable macOS CI, after the pre-existing original PRN test prepares it.

**Original-derived C code:** `_FlushDataStreams` (original macOS address `0x1000015f0`) was recovered from the verified original behavior, also independently confirmed by the supplied original Linux 1.0.3 `raster_c_functions.c`. `classic_original_raster_flush_streams` first flushes stdout and then flushes the optional PRN dump. It ignores both return values, as the OEM did. The joined macOS CUPS candidate now calls this reconstructed routine instead of an inline duplicate. Standalone tests verify both flushed file descriptors and the missing-dump branch.

[Full original comparison and compiler run 38574758345](https://github.com/an3k/leitz-icon-linux/actions/runs/38574758345): three successful CI jobs, 13/13 existing full-command cases and 9/9 native joined-raster output cases still byte-identical.

Implementation commits: `0c8373a083473442a6c0bb72b4e38c11f5da1eeb` (additional original comparisons), `a278e0e0f683aa94381d342b7b5deef26e6b624c` (OEM flush function integration).

The original native global ABI, untested memory/error/network branches and real printer/cutter operation remain outside the proof. The Linux driver, CUPS branch, originals and enhancements were unchanged.

## 2026-10-14 — exact native stderr parity restored for OEM raster modes 2 and 3

The live `sources` branch was inspected at `0b84c92d6cb6f91c810bf6e3e427a6c16e8dd85e` before modifying code. The joined OEM-derived macOS CUPS executable, exact stdout tests for four two-page jobs, extended variable-length and PRN tests, and original command status code were already complete for their documented scenarios. Existing byte encoders and vendor inputs were not replaced.

The original comparison script `verification/macos-5.1.465-classic/compare_connected_with_vendor.sh` now requires matching **complete stderr bytes**, as well as binary stdout and exit status, from the authenticated original raster2printer and the joined reconstructed CUPS executable. The first run [38591813876](https://github.com/an3k/leitz-icon-linux/actions/runs/38591813876) identified two genuinely missing OEM cases, with unchanged printed bytes:

1. `_DitherPage` source-line 594 logs the **packed monochrome output stride** (88 for a 704-pixel RGB24 row) rather than the 2112-byte RGB24 input stride. `original_dither_page.c` now reads the existing `descriptor->packed_bytes_per_line` for that diagnostic, leaving the actual pixel converter's RGB input stride unchanged. Its unit-test fixture has independent packed and RGB strides.
2. In optimization mode 2, `_ProcessPage` emits original source-line 860 message `first bit/byte. do not send 255` at the original first-bit sentinel. Disassembly of the verified original `_ProcessPage` established `0x10000825b` initializes the byte-run counter to `0xff`, `0x100008262` initializes previous bit to zero, and `0x1000083e5..0x100008414` prints the diagnostic if the **first bit** of the cropped nonwhite row is one. The run accumulator is then reset; further run lengths trigger the 0x7f cap, so the diagnostic is **not** repeated for later runs. The first attempted all-128-one-run model was corrected using the newly collected native instruction evidence, not by hardcoding test-specific counts. `original_raster_process_page_modes23.c` emits this diagnostic separately without touching the already byte-identical row encoder.

**Verified:** [GitHub Actions 38593969771](https://github.com/an3k/leitz-icon-linux/actions/runs/38593969771) completed all three native/Linux CI jobs successfully and confirmed four original-vs-connected two-page jobs (monochrome + RGB24, modes 2/3) with **exact stdout bytes, complete stderr bytes, and identical exit status**. The existing four variable-raster-height, original PRN, and 13 connected command-response comparisons also remained byte-identical. A separate mode-2 unit-test assertion additionally checks the number of original sentinel diagnostics against independent synthetic rows.

The manufacturer x86_64 binary ABI, modes 0/1 undefined final-row reads, live hardware/network communication and unexamined error cases remain incomplete. Neither the Linux driver nor the OEM originals were changed.

## 2026-10-14 — original raster utility symbol linkage in joined macOS CUPS candidate

The actual `sources` HEAD `ae1a7b00e9e20e7320609f9f3051cebcf519477e` and prior successful OEM comparisons were inspected before writing. The OEM model implementations already existed and were **not rewritten**. The newly connected C source is `reconstructions/macos-5.1.465-classic/raster2printer/original_raster_cups_candidate.c`.

**Reconstructed original-named function boundaries:** `OpenDebugFile`, `CloseDebugFile`, `WriteDebugBuffer`, `SendDebugFileToPrinter`, `FlushDataStreams`, `is_bit_set` and `WriteData`. Existing named `OpenLogFile` and `CloseLogFile` remain connected. The named functions reuse the existing OEM-derived PRN, log, byte-output and x86 bit-test implementations, preserving the original fixed debug paths and output choices. Active joined-job callbacks now dispatch through these named boundaries. The `WriteData` boundary receives the already copied **0x10a8-byte descriptor** and selects exactly the original single socket write versus stdout and optional PRN write; it is a source-level C bridge, **not** the manufacturer's binary-compatible C++ signature.

The native macOS CI job now checks actual exported symbols with `nm`; the independent Linux test suite continues compiling/running the original-derived helpers. [Source branch CI run 38598019684](https://github.com/an3k/leitz-icon-linux/actions/runs/38598019684) passed all three CI jobs, including original SHA-256-verified Mach-O comparisons. The original-versus-joined candidate remained exactly identical for four entire mono/RGB24 print jobs (stdout, stderr and exit status), four additional variable-height raster cases (stdout and exit), one original PRN file-transfer case (PRN file, stdout and exit), and 13 full command status cases (stdout, stderr, exit). Original mode-0/1 output, socket/IPP and untested errors are not established by these cases. Code commits: `9c6533e786fd8a0685f368c7727d9224cc065be3` and `93946f2a5e861ab4bba8c01f2def9d4b4b8dde0f`.

These entry points do **not** assert reproduction of the original private C++ symbol ABI, global memory layout, unchecked socket failure paths, OEM mode-0/1 last-row overread or physical hardware behavior. This branch is reference reconstruction only; no productive `driver`, `cups` or manufacturer-original sources were edited.

## 2026-10-14 — both native macOS executable candidates now link recovered dormant diagnostics

Checked live `sources` at commit `9f80cbe2e7a2bd1c51bc8a403649b47c6b299881` before changing anything. Three OEM diagnostic functions `output_cups_info`, `test_raster`, and `PrintPageDebugInfo` were already reconstructed in `reconstructions/macos-5.1.465-classic/shared/diagnostics/`, with independent Linux GCC/Clang and native macOS SDK compilation. Previously **neither of the two joined native CUPS candidates linked those existing bodies**. Their sources are now used directly by both Makefile targets, without copies or rewritten implementations. They remain dormant, exactly as in the normal OEM executable control paths. Their original unchecked PPD/row behavior has not been hardened, and they are not production-safe.

The original macOS `command2leitz` candidate had implemented status query and ESC Z algorithms without matching independent source-level exported function names. The existing implementation is now callable using `GetStatus`, `GetPrinterStatus`, `GetUnitedStatus`, `GetFirmvareVersion` (original OEM spelling), and `EndOfJob`. OEM main now selects the corresponding named query entry, which reuses a **single** recovered `GetStatus` body. Separately, the existing `classic_command_write_data.c` (OEM socket/PRN/stdout and unusual signed return semantics) is linked and exposed as dormant `WriteData` in the command candidate. This is **not** a change to active command bytes or an invented status query.

The source-level C wrappers use injected callback structures, **not** the binary's original private C++ calling convention. Existing isolated command tests now call each new named status/ESC-Z entry directly. The native macOS build checks all six named command entries and all three dormant diagnostic names in the compiled command program, plus the corresponding three dormant diagnostic symbols in the compiled raster program, using `nm`.

**Verification:** [GitHub Actions run 38604431098](https://github.com/an3k/leitz-icon-linux/actions/runs/38604431098) passed all three jobs: Linux GCC/Clang recovered C tests, native macOS Clang compile and symbol checks, plus checksummed original macOS Mach-O side-by-side comparison. The connected reconstructed command entry retained **13/13** complete identical stdout/stderr/exit cases. The joined raster entry retained **4/4** stdout+stderr+exit matches, **4/4** variable-height stdout+exit matches, and one PRN file+stdout+exit match.

Implementation commits: `0a30ec3a5d96d51d7350595cfa0e66b5f544bd26` (shared OEM diagnostics linked to both), `b5a4929b1c8a8e5d4bd5040ed9d5cb1eda8ea070` (named command entries linked), and `be7be990f7a12bff558cbe84dc9d4353b499f827` (direct original-entry test additions).

**Still not reconstructed to native binary equivalence:** the manufacturer's original private C++ ABI/globals, the four-byte end-of-buffer read in modes 0/1, incomplete IPP/network state and hardware behavior, and still unexamined error branches. The original Linux production driver, `driver` and `cups` branches, and OEM original archives were not altered.

## 2026-10-14 — macOS OEM startup errors and dormant status/URI functions

**No duplication:** Inspected the live `sources` branch at `d734eec6c9e1394220a2a3b6b985c4043558bb93` before continuing. The original preflight, CUPS raster/error behavior and `original_status_exchange.c`/`original_device_uri.c` implementations already existed. No original encoder, installed Linux driver, or vendor file was replaced.

**Real original start/error path coverage:** New `verification/macos-5.1.465-classic/compare_connected_startup_errors_with_vendor.sh` runs the independently linked C raster candidate and checksum-authenticated original macOS 5.1.465 Mach-O with identical environment and inputs. [CI run 38605605549](https://github.com/an3k/leitz-icon-linux/actions/runs/38605605549) passed exact complete **stdout bytes, stderr bytes and exit codes** for six additional original `_main`/`_raster2printer` branches:
- argc 5: exit 1; no stdout.
- argc 8: exit 1; no stdout.
- Nonexistent input raster: exit 1; no stdout.
- Nonexistent PPD: exit 255 (manufacturer returns -1); no stdout.
- Zero-byte input raster file: exit 1; no stdout.
- Empty standard input when argc==6: exit 1; no stdout.

This establishes those six defined original startup/error behaviors with no further changes to existing correct code.

**Source-level OEM status/URI integration:** The previously recovered `classic_original_check_status` and `classic_original_resolve_device_uri` implementations now link into the joined macOS executable with manufacturer-named **`CheckPrinterStatus`** and **`ResolveDeviceURI`** function boundaries. Original CUPS `cupsBackChannelRead`, `send`, `recv`, `fflush(stdout)`, `putchar`, `sleep`, the existing OEM `WriteData` sink, global job counter and original socket/URI state are connected via C callbacks. The original 0x10a8-byte descriptor offsets and copied reset payload remain in use. The existing isolated status tests still cover media missing/door/no cartridge/ready/backchannel errors, an IPP-socket path and early resolver; the already recovered URI test only uses a **non-IPP URI**.

**No speculative printer behavior:** Original `_raster2printer` does not call `_CheckPrinterStatus` in the verified ordinary print loop, so the connected functions remain **dormant** there. The URI routine retains the original real TCP/9100 connection behavior if explicitly invoked for `ipp`, but that path was **not run** against a remote or actual printer. Manufacturer socket failure cases, native compiler ABI/global layout and analyzer-only status `-2` undefined-case markers are still incomplete.

**Verification:** [GitHub Actions 38606149312](https://github.com/an3k/leitz-icon-linux/actions/runs/38606149312) passed all three jobs: Linux GCC/Clang tests, native Apple CUPS compile and symbol checks for both function entries, and SHA-256-authenticated OEM comparison. It retained **6/6 startup/error**, **13/13 command**, **4/4 whole-raster stdout+stderr**, **4/4 variable-length stdout** and **1/1 PRN-file** exact comparisons with the manufacturer. No physical printer, USB or TCP device was contacted by these new tests.

Code commits: `bc9dd2a910e0d31b4abb773dbe99db195a9ab7cc` (start/error OEM comparisons), `f7a1dfcb01d4fc8002435423c0ea1acdb04b5d93` (OEM status/URI linkage) and `155a5b2e94418938e6bf22124869d04eb0ce6918` (native CUPS declaration). The two original 5.1.465 executables, `driver` and `cups` remain unchanged.

## 2026-10-14 — native RGB24 mode-1 original parity with defined final-row data

The current sources HEAD `17d4a2d14809477f23ba405836adeb9546ca5d77` was inspected before this increment. The previous commits already connected original-named CUPS document/page/dither and `ProcessPage` entry points and established original manufacturer mode-1 parity for all-blank and mixed monochrome rows. None of those functions were rewritten.

A still-unchecked whole-job combination was OEM optimization mode **1** with RGB24 input and original `_DitherPage`. The already existing CUPS fixture generator gained a separate `rgbtailblank` input mode: normal original RGB gradient content in all but the last row and a completely white RGB final row. This preserves the original mode-1 +4-byte lookahead semantics while avoiding a last-row allocation overread during actual vendor execution. The normal `rgb` and `tailblank` fixtures remain unchanged.

`verification/macos-5.1.465-classic/compare_mode1_rgb_with_vendor.sh` now runs the independently compiled reconstructed joined CUPS executable and SHA-256-verified original macOS Mach-O on identical two-page RGB24 raster jobs. It checks **all stdout bytes, all stderr bytes and exit code**, independently for two heights and two cutter settings:

| Raster height (rows, not millimeters) | Cutter | Original stdout bytes | Result |
|---|---|---:|---|
| 33 | ON | 6,052 | Exact match |
| 33 | OFF | 6,050 | Exact match |
| 80 | ON | 14,700 | Exact match |
| 80 | OFF | 14,698 | Exact match |

[CI run 38611488870](https://github.com/an3k/leitz-icon-linux/actions/runs/38611488870) passed all three jobs: recovered C under GCC/Clang, native Apple CUPS compilation, and actual authenticated manufacturer binary comparisons. All four new cases emitted `OEM_CONNECTED_MODE1_RGB_PARITY=YES`. Existing monochrome, command, PRN, original modes 2/3 and startup comparisons remained successful.

Commit: `1e330ac0ef862eafd7ab5b279d35bd38b1b542c5`. This establishes four more original-equivalent defined end-to-end scenarios, **not** complete mode-0/1 equivalence for nonblank final rows, native C++ binary ABI equivalence, network behavior or physical printer functionality. The productive driver, CUPS branch, and original binaries were not changed.

## 2026-10-14 — all remaining RGB24 quality tables and CUPS IPP-route reference

Live `sources` was checked at `b39862736a1dd06884dac249ebe847c33af177c0` before modifying any files. The original-derived connected macOS CUPS candidate and all 15 previously recovered PPD quality-name mappings already existed. The valid `Bayer8x8` RGB24 baseline and separate original `Draft` (1-bit input) handling had already been verified. The original quality matrix code was not rewritten.

**Complete, independently executable manufacturer RGB24 comparison:**
`verification/macos-5.1.465-classic/compare_connected_halftones_with_vendor.sh` uses separate SHA-256-verified original manufacturer and reconstructed macOS CUPS executables, identical two-page RGB24 input, actual PPD selections and USB-oriented `DEVICE_URI`. It compares **entire stdout bytes, entire stderr bytes and exit codes** in modes **2/3**, at **300/600 dpi**, for all **13 other non-Draft original quality names**:
`Bayer8x8SuperCell1`, `Bayer8x8SuperCell2`, `SuperCell1`, `SuperCell2`, `SuperCell3`, `CUPSThresold64x64`, `SuperCell`, `Bayer4x4`, `Bayer14x14`, `Clustered8x8`, `Dispersed8x8`, `Nasik4x4`, `Dithering3x3`.
[Run 38623180688](https://github.com/an3k/leitz-icon-linux/actions/runs/38623180688) confirmed **52/52 exact whole-job matches** and all three CI jobs successful. These were formerly only partly tested using the older standalone raster adapter, not the original-function-connected executable. No encoder correction was needed.

**IPP scheme in a strictly local CUPS raster process:**
`verification/macos-5.1.465-classic/compare_connected_ipp_loopback.py` executes the original manufacturer and joined reconstructed CUPS binaries separately with identical `DEVICE_URI=ipp://127.0.0.1/ipp/print`. For each process a new TCP listener binds only `127.0.0.1:9100`, and a synthetic `SKU:Ready;` response is provided only if a command actually arrives. This test never contacts remote hardware.

[Run 38623604108](https://github.com/an3k/leitz-icon-linux/actions/runs/38623604108) matched original **stdout bytes (3,620), complete stderr bytes (9,404), exit status (0) and TCP bytes (0)** against the joined reconstruction. The **original itself used stdout**, despite the IPP URI; the reconstruction correctly preserved that route rather than inventing a TCP connection. That same run reconfirmed all 52 RGB24 comparisons and all other existing CI jobs.

Limits: these experiments do **not** verify a real TCP/9100 printer connection, socket error/retry paths, the dormant `CheckPrinterStatus` function with a printer, the native C++ ABI or the original's undefined mode-0/final-row mode-1 overreads. They change neither installed Linux driver nor the original vendor binaries. Commits: `1d7c65c9f4859cd5e4eddb1c9fbc515461481cc5` (quality coverage) and `86d0f8e6622f1b956ffbd9d353876968d8427226` (local IPP-route comparison).

## 2026-10-14 — original command sequences and mode-0 defined-byte parity

Before this increment, `sources` was verified at `43573c9de5509cff153e079d794c6e08ff5206f1`. The joined original-derived macOS raster and command implementations were already present. Existing reconstructed C source functions and manufacturer files were **not rewritten**.

**OEM command2leitz multi-command control:** The established original status executable comparison was extended to six previously unverified multi-command configurations: `GetUnitedStatus→GetFirmwareVersion`, reverse order, `GetUnitedStatus→ReportStatus→GetFirmwareVersion` (stops at ReportStatus), `ReportStatus→GetUnitedStatus` (stops immediately), case-insensitive query names, and unknown instruction followed by GetUnitedStatus. These are real CUPS command-file reads with the same synthetic backchannel for independently launched original and reconstructed Mach-O/C executables. [CI 38624982713](https://github.com/an3k/leitz-icon-linux/actions/runs/38624982713) confirmed **19/19 complete matching command cases** (all stdout bytes, stderr bytes and exit status). Six new cases were verified, and no duplicated C status algorithm or speculative behavior was introduced.

**Original optimization mode 0:** The original `_ProcessPage` mode 0 copies `packed_stride+4` bytes from every raster row; its final four bytes of each page are **outside the CUPS raster allocation** and have undefined process-memory contents. The existing `compare_defined_bytes.py` helper explicitly ignores exactly four bytes immediately before the original per-page footer, and checks every other byte. The native `compare_connected_mode0_defined_with_vendor.sh` compares the independently linked joined CUPS candidate against checksum-authenticated original manufacturer executable with two-page monochrome and RGB24 files, heights 33 and 80 CUPS **raster rows**, and cutter ON/OFF (8 combinations). It requires equal process exit codes, **entire stderr bytes**, equal stdout lengths, matching page boundaries and all defined stdout bytes. It does NOT claim equality for eight undefined bytes per two-page job.

The first run revealed a **verification-tool-only** omission: with cutter OFF, the original EndPage command is `ESC G` (two bytes); with cutter ON it is `ESC G ESC E` (four bytes). The matcher was extended with an explicit `--cutter-off` choice and its length-dependent byte slice/range corrected. Neither original C encoder, nor the CUPS filter, nor the OEM binaries were altered.

[CI 38625963144](https://github.com/an3k/leitz-icon-linux/actions/runs/38625963144) passed all three jobs, including **8/8 defined-byte mode-0 manufacturer matches**, entire diagnostics and exits, plus **19/19 command cases**. The eight intentional, vendor-undefined bytes per two-page job remain excluded from full-stream equality; no fixed label length is implied, no physical printer or network device was contacted.

Code commits: `7cf431df587901f9818a2d189fb693826ed6e75b` (multi-command OEM comparison), `8fdcef86450a36a3f57ee5883deb8ec84eea9d69` (mode-0 comparison), `cf7825c2534d43a9d361ad3a092a89e2a3dd0086` (explicit no-cut delimiter), `92b285689ffb44d5bf1d460356385efb37ea3e3c` (variable-length page-boundary matching).

Remaining: original native C++ by-value ABI/global-state parity, unchecked socket/IPPs, safe interpretation of manufacturer final-row out-of-bounds reads and real hardware/USB/TCP printing. The original vendor archives and productive `driver`/`cups` branches were unchanged.

## 2026-10-14 — original command ShouldSend_ESC_A option matrix

The live `sources` branch was checked at `2ad4859a1988b8f809ad8ba9e115009a7a6d0d07`. Existing recovered status algorithms, 19 previous exact macOS manufacturer command cases and original PPD `ShouldSend_ESC_A` options were already present and were not recreated. The OEM PPD defines `ShouldSend_ESC_A=YES` as its default, with a distinct `NO` selection. The reconstructed `EndOfJob` originally ignored its argument, but this CUPS option behavior had not been verified through the **complete original executable** for all three commands.

`verification/macos-5.1.465-classic/compare_connected_command_with_vendor.sh` now exercises explicit `ShouldSend_ESC_A=YES` and `ShouldSend_ESC_A=NO` for `ReportStatus` (READY), `GetUnitedStatus` (U_42) and `GetFirmwareVersion` (V_42) against independently launched, checksum-verified original macOS `command2leitz` and the joined reconstructed CUPS candidate. Both run with the same original PPD and instrumented synthetic CUPS backchannel. Every comparison requires **complete stdout, complete stderr and exit code equality**, and confirms the backchannel interposer ran in both binaries. No real printer was contacted.

[Run 38627418640](https://github.com/an3k/leitz-icon-linux/actions/runs/38627418640) passed all three CI jobs and **25/25 full original-command equality cases**, including all six newly checked PPD option cases. Both OEM and reconstructed `EndOfJob` behavior agree for YES and NO in the checked conditions, so the existing recovered C source was left unchanged. Other PPD options, native C++ ABI and genuine hardware remain outside this proof. Commit `9f36faecb269f63670647126fdaeb9e64f7d38e8`.

## 2026-10-14 — exact native signal-handler symbol and command PPD option comparison

The actual `sources` branch was inspected at `2ad4859a1988b8f809ad8ba9e115009a7a6d0d07` before execution. The original macOS 5.1.465 C signal handler's diagnostic, termination global assignment, SIGTERM registration and all previous observable-output cases were already reconstructed and were not replaced.

**Newly recovered native symbol binding:** The manufacturer exports the C++-mangled Mach-O symbol `__Z14signal_handleri` (original address `0x100004420`). The existing reconstructed `signal_handler(int)` uses the same single-integer calling convention. Its header now explicitly names the original `__Z14signal_handleri` assembler symbol **only on Apple**, while retaining the portable C name for Linux. `original_raster_main.c` and its isolated test include this same header, ensuring both the normal handler registration and the test mock resolve the OEM symbol. An initially detected native test-link mismatch was fixed in the test's declaration, without duplicating the function body.

**Proof:** [GitHub Actions 38628189356](https://github.com/an3k/leitz-icon-linux/actions/runs/38628189356) passed Linux GCC/Clang, native macOS Clang and its explicit `nm` requirement for the C++-mangled export, plus the authenticated original manufacturer execution suite. Among the retained original-verified comparisons: **25/25 complete command**, **52/52 RGB24 halftone**, **4/4 connected normal raster**, **4/4 connected mode-1 RGB24**, and **8/8 mode-0 defined-byte** cases. No print protocol or ordinary stream was changed. The original C++ private global-memory layout and the mode-0/1 undefined last-row reads remain outside this proof.

Changes: `a8cf31d182e7eeee0ece0bd2ecfa116fc064fb9d` (native OEM symbol) and `b39127a704fad9d0ea6174ed3ef0794b49d26952` (matching isolated mock declaration).

## 2026-10-14 — native reconstructed URI TCP/9100 loopback execution

Prior to execution, the actual `sources` branch was checked at `b6453aa258014aa9484f5a605689144f5f0bb4f5`. The original-derived `classic_original_resolve_device_uri` implementation already contained the OEM `cupsBackendDeviceURI`, `httpSeparateURI`, `gethostbyname` and `connect` operations and fixed TCP destination port `0x238c` (9100). No reconstruction logic or installed printer driver was modified.

**Previously untested branch executed:** Added `original_device_uri_loopback_test.c` as a separate native Apple CUPS test, with a socket listener bound explicitly to **127.0.0.1:9100** on an ephemeral macOS runner. The test passes `DEVICE_URI=ipp://127.0.0.1:4231/printers/icon` into the actual recovered URI resolver. It requires the recovered descriptor scheme `ipp`, hostname `127.0.0.1`, resource `/printers/icon` and preserved URI port `4231`, then verifies via `select`, `accept` and `getpeername` that the outgoing connection goes **only** to local TCP port **9100**, regardless of the explicit different URI port. Accepted and client sockets are closed before exit. The pre-existing isolated non-IPP URI test and normal printer path are unchanged.

**Evidence:** [GitHub Actions run 38628907264](https://github.com/an3k/leitz-icon-linux/actions/runs/38628907264) passed all three CI jobs. The native macOS job reported `OEM ResolveDeviceURI loopback connection to fixed TCP/9100 passed`. The already checksum-authenticated manufacturer raster/command regression suite also succeeded. This is a **real local TCP connection through the recovered C function**, **not** a side-by-side execution of the dormant OEM Mach-O `_ResolveDeviceURI` routine and not proof of live device communication. OEM native ABI and genuine hardware/socket failure behavior remain incomplete.

Code commits: `4f8eeee7460e1327269c321faed83ca0a633159f` (loopback fixture) and `a9d60c2b57c0736db34e45624fe2c2a27333f8be` (macOS SDK-compatible loopback address comparison).

## 2026-10-14 — native status global-state rereads recovered

The live `sources` branch was inspected at `88a8ce41ebca3b08ab94f30e2dbe207ca89aa0b6` before work. The existing offline native macOS `_CheckPrinterStatus` implementation was already recovered and linked, but its callback C adapter snapshotted the original device socket and resolved URI pointers once on function entry. The verified original 5.1.465 disassembly reads the OEM `__ZL4sock` and `__ZL12resolved_uri` globals **each time** it checks its IPP gate, sends ESC U, receives the reply, or forwards the media-reset four-byte `_WriteData` command. That original dynamic state could not be represented by the previous one-shot snapshot.

**Completed correction:** The existing original-status callback interface now includes optional `current_socket_fd` and `resolved_uri_is_present` readers. The macOS CUPS reconstruction binds them directly to the existing `classic_original_socket` and `classic_original_resolved_uri` globals. The recovered original status loop accesses the fresh values at the manufacturer-confirmed sites. The legacy static values continue working for standalone tests. The original ESC U bytes, socket flags, 64-byte response, 5-second retries, 7-second backchannel, 0x10a8-byte descriptor copy, four-byte media reset and original error texts are unchanged. No new attempt to connect to hardware or change the standard printing route was added.

**Reproducible additional branches:** New isolated regression cases start with an IPP socket that becomes unavailable during the original five-second retry callback. The next loop must observe the current -1 socket and call `ResolveDeviceURI` exactly once, without sending on the stale descriptor. Another case proves that live globals override stale -1/null initialization snapshots. The initial GCC `-Werror` compile detected a missing optional-field initializer in the test fixture, corrected in the next commit; no production change was involved.

**Verification:** [Actions run 38635127943](https://github.com/an3k/leitz-icon-linux/actions/runs/38635127943) passed all three jobs: recovered C under GCC and Clang, native macOS Clang and linked symbol checks, plus the checksum-authenticated full original manufacturer regression. Previous behavior remains exact for **25 command cases, 52 RGB24 matrix cases, four complete normal raster cases, four mode-1 RGB24 cases, eight defined-only mode-0 cases, six startup/error cases and one local CUPS IPP stdout-route comparison**, with no failed manufacturer comparisons. This verifies the new *isolated* dynamic-state logic against the recovered disassembly and unit tests, **not** against a manufacturer invocation of dormant `_CheckPrinterStatus` or a real printer. Original native private C++ ABI, unchecked socket failures and mode-0/1 undefined buffer bytes remain open.

Commits: `656a38c1c0c01046c3236482b30b154b4ae404e7` (live-global callbacks and connected function), `859bcce2696bfbebf363f66567e3a020709be74e` (initialized test fixture).

## 2026-10-14 — recovered OEM private Mac globals and C++ helper symbol names

Before these changes the actual `sources` branch was inspected at `5f09c0ef87445161bea42fefcf1ff5e6f6e75542`; the prior live socket/URI getter correction, isolated status tests and checksum-authenticated manufacturer CUPS comparisons already existed and were not repeated in source form.

The original x86-64 Mach-O disassembly of `_CheckPrinterStatus` and `_ResolveDeviceURI` identifies file-private globals **`__ZL4sock`** and **`__ZL12resolved_uri`**. The existing shared reconstructed `classic_original_socket` (initialized -1) and `classic_original_resolved_uri` (initialized NULL) are now bound to those exact assembler symbol names **on macOS only**, without changing the resolver, status logic, initial values or linked global instances. The existing original `_terminate_flag` and `__Z14signal_handleri` link paths remain unchanged.

The SHA-256-authenticated Mach-O inventory also identifies five original file-private utility functions. Their previously recovered C bodies now carry the original macOS symbols: **`__ZL7strrchrUa9enable_ifIXLb1EEEPKci`**, **`__ZL6strstrUa9enable_ifIXLb1EEEPcPKc`**, **`__ZL12_OSSwapInt16t`**, **`__ZL16__libcpp_strrchrPKci`** and **`__ZL15__libcpp_strstrPKcS0_`**. The two internal libc wrapper bodies are marked `used,noinline` on macOS to retain their OEM-named definitions for symbol inspection; no additional calls were injected into the actual CUPS job. Normal Linux test builds retain their original portable C names.

[CI run 38636373462](https://github.com/an3k/leitz-icon-linux/actions/runs/38636373462) passed all three jobs for the original-named globals. [CI run 38636640251](https://github.com/an3k/leitz-icon-linux/actions/runs/38636640251) passed native macOS and Linux compilation plus the three externally visible original helper names. The remaining internal symbols are independently checked by the final native `nm` run after this commit. All these are **source-level symbol/ABI progress**, not a claim of fully identical original C++ object visibility, compiler-generated instruction sequences, original status-function ABI, mode-0/1 undefined memory behavior, or real hardware communication. Original vendor files, `driver` and `cups` branches were not modified.

## 2026-10-14 — OEM private job counter and actual FILE pointer globals

Before editing, inspected the live `sources` branch at `25ef9a875da016e05426d2e9dba25c3533e97b2a`, inspected previously successful CI, and confirmed the recovered macOS raster _main, _CheckPrinterStatus, OpenDebugFile, OpenLogFile and _WriteData existed. No completed algorithms were reimplemented.

**Actual 32-bit OEM job counter:** SHA-256-verified original _CheckPrinterStatus disassembly at `0x100009713..0x10000971f` increments the file-private Mach-O symbol `__ZL8job_size`. The joined CUPS candidate formerly declared a plain global `job_size` while _main separately used its own ordinary C declaration. The native definition and common _main/mock-test declaration now bind the same **one** 32-bit counter to `__ZL8job_size` on macOS, preserving the original source-level job accounting. An initial native link run exposed the inconsistent external `_job_size` reference; common-header correction resolved it without changing counter arithmetic. [CI run 38637865889](https://github.com/an3k/leitz-icon-linux/actions/runs/38637865889) passed all three jobs with the native symbol check and actual original comparison.

**Manufacturer FILE* globals rather than a synthetic Boolean:** Verified original _WriteData x86 instructions `0x100001b28..0x100001b58` check and pass **`__fd_stdout`** as a real pointer to fwrite. The joined CUPS candidate now defines the OEM-named native FILE pointer, initializes it to stdout at job start, sets it to stdout/NULL through the existing PPD `PrintThroughPRN` callback, and uses it as the fwrite stream and gate. Its earlier `stdout_enabled` integer was removed; the recovered write/core algorithm and manufacturer's separate PRN behavior were not replaced. The original `__fd_dump` and `__fd_log` FILE pointers now also have native global symbols and are synchronized at the existing recovered `OpenDebugFile`, `CloseDebugFile`, `WriteDebugBuffer`, `FlushDataStreams`, `OpenLogFile` and `CloseLogFile` boundaries. These continue to use the previously verified original PRN/log body implementations and paths; no duplicate file routines or extra printing calls were introduced.

**Verification:** [GitHub Actions 38638374211](https://github.com/an3k/leitz-icon-linux/actions/runs/38638374211) completed all three CI jobs successfully. Native macOS `nm` checked `__ZL8job_size`, `__fd_stdout`, `__fd_dump`, `__fd_log`; GCC/Clang tests passed. Authenticated original manufacturer full-process comparisons preserved **25/25 command**, **52/52 RGB24 halftone**, **4/4 complete mono/RGB mode-2/3**, **4/4 RGB24 mode-1**, **8/8 defined-byte mode-0**, **6/6 startup/error**, **1/1 PRN** and **1/1 local IPP-stdout route** cases. No hardware was contacted. Native C++ by-value descriptor ABI, compiler memory layout, undefined last-row reads and untested USB/TCP failure/recovery remain incomplete.

Code commits: `b1d73ac62c97ac0bdefbfc205918b265189b616d` (original counter binding), `cb6c641a278dc7c56056c556d0137a9e9f424744` (common declaration), `dfd9e963b7f3f6c64db5842cd1efebb2d2280ae4` (original stdout pointer) and `d3d7491e4e2db0f8d8a08ac4b0f27229b93fb3bb` (original log/PRN pointer linkage). Only `sources` was changed.
