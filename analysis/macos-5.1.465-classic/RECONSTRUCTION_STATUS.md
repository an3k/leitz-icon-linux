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
