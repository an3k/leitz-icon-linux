# macOS 5.1.465 classic — reconstruction state

## Manufacturer inputs (unmodified)

- `raster2printer`: Mach-O x86_64, 84,252 bytes, SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`.
- `command2leitz`: Mach-O x86_64, 35,932 bytes, SHA-256 `6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee`.
- `LeitzIcon.ppd`: unmodified manufacturer PPD.
- The original archive does **not** contain original C files for either executable.

## C source availability

**Neither executable has a complete faithful C reconstruction.** The existing symbol lists and assembly analyses describe specific functions but cannot establish the original C source, all code paths, or full binary-to-C equivalence.

Experimental, compilable C fragments formerly kept in `reconstructed/` have been moved unchanged into `enhancements/macos-5.1.465-classic/reconstructed/`. Do not attribute these files to the manufacturer or use them as exact replacements.

The fragment library, command filter and test executables were built using GCC on Linux (`make clean all check`, successful); the full CUPS raster adapter could not be compiled in the local environment because genuine CUPS development headers are unavailable. The original Mach-O executables have not been run for reference-output comparison.

## Documented intentional divergences in the experiments

- Rejecting stdout/PRN flush errors where the original routine does not check the return code.
- Extending the portable raster-header callback to distinguish negative errors from end-of-file.

Further differences may exist. Neither the successful builds nor local smoke tests establish equivalent printing or cutting behavior.

**Before a 1:1 claim:** reconstruct every active code path for both executables; compile for macOS using compatible system APIs; compare identical inputs/options, device-output bytes, status exchanges, errors and edge conditions against the original executable. No such final comparison is available.

## Additional recovery committed on 2026-10-13

- `recovery/command2leitz_recovered.c` plus `command2leitz_cups.c`: independently recovered active command-dispatch and CUPS entry path, built and exercised offline. Original execution and full macOS equivalence remain outstanding.
- `recovery/raster/`: five raster lifecycle/output boundary functions reconstructed separately from assembly; built and checked offline. The raster filter as a whole remains incomplete.
- Existing experimental C stays isolated under `enhancements/`, and the manufacturer originals remain unmodified.

## 2026-10-13 — recovered manufacturer halftone values

- All 13 matrix tables and all 5237 integer elements previously extracted from the classic Mach-O are now present in standalone C under `recovery/raster/`. C declarations expose these arrays without changing their numeric values.
- All 15 original `PrintQuality` labels were matched against the byte-identical original PPD, and their previously reverse-engineered selector indices, matrix sizes and scaling constants are in separate C.
- Full `_DitherPage` and full raster filter remain incomplete; no 1:1 equivalence claim. This revision has **not** been newly compiled because the container runtime was unavailable.

## 2026-10-13 — dither inner loop reconstruction

Added `recovery/raster/raster2printer_dither_recovered.[ch]`, extracted from the already documented OEM `_DitherPage` RGB24 arithmetic. The recovered fragment uses manufacturer matrix values and exact previously decoded mode selection, without previous experimental boundary checks; additional test inputs have been added to the offline Makefile. **The complete original function is not recovered, and no compiler or OEM execution check was possible in the current session.**

## 2026-10-13 — raster input and output recovery

New standalone C in `recovery/raster/`: `classic_read_page.[ch]` and `classic_write_data.[ch]` with explicit callback boundaries. These correct two differences from previously hardened project code: page index increments at header acceptance and nonzero short page reads remain successful; short socket sends are not automatically retried. Original branching evidence is documented by original Mach-O virtual addresses. No full ABI, complete job path or binary-equality assertion is made. GCC/Clang CI build verification is configured separately and must not be reported as passing until the workflow concludes successfully.

## 2026-10-13 — ProcessPage modes 0/1 independent recovery

Added `recovery/raster/classic_process_page_modes_01.[ch]`: original 0/1 raster row header and row forwarding, including the original deferred empty-row handling in mode 1. The standalone ABI is project-authored; a full equivalent `_ProcessPage` including 2/3, global state and all branches is **not yet available**. GCC/Clang verification is part of the sources-only workflow and must be checked against the completed run for the associated commit.

## Verified source build: 2026-10-13

For commit `1345811cea4198e52554e8f3e0ac0738ceea2fd3`, workflow [Sources - Reconstruction Compile](https://github.com/an3k/leitz-icon-linux/actions/runs/38379977044) **completed successfully**: GCC and Clang built and exercised both isolated command and raster recovery test suites. This confirms **compilation and local test results only**, not executable parity with the original macOS Mach-O files or hardware behavior.

## 2026-10-13 — mode 2/3 row path recovery

Added `recovery/raster/classic_process_page_modes_23.[ch]`, based on the classic OEM `_ProcessPage` branches at `0x100007a74..0x100008882`. It recovers row-level original mode-2 bit runs, strictly shorter compressed selection, mode-3 edge treatment, dynamic offsets/widths, deferred blank rows, page-persistent width caches, and the distinct explicit reprint exception. The existing mode 0/1 source and `enhancements/` code were not changed. The full manufacturer function and original Mac binary parity remain unresolved. A GCC/Clang build workflow is triggered on this new commit; record actual outcome separately.

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
   independently as `recovery/raster/original_device_uri.[ch]` using
   original descriptor offsets, `cupsBackendDeviceURI`,
   `httpSeparateURI`, IPv4 socket setup and **fixed TCP port 9100**
   regardless of URI port. The original keeps a socket failure and null
   DNS pointer defect; these have not been silently improved.
2. `_CheckPrinterStatus` `0x100009190..0x100009a2e` was disassembled.
   `recovery/raster/original_status_decoder.[ch]` now records its
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
- Newly reconstructed `recovery/raster/original_status_gate.[ch]`
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

A fresh, SHA-256-verified macOS Intel reference run captured the actual `command2leitz` machine instructions. Four previously unimplemented helpers now have isolated C in `recovery/classic_command_original_utils.[ch]`:

- `_is_bit_set` at `0x100001c60`: original 32-bit bitmask comparison, including x86 shift-count masking;
- `_FlushDataStreams` at `0x100001c20`: unconditional output flush and optional debug flush, with error results ignored;
- `_CloseDebugFile` at `0x100002440`: conditional fclose and clear the file pointer;
- `_CloseLogFile` at `0x100002480`: conditional fclose and clear the file pointer.

The callback/pointer interfaces are separate reconstruction scaffolding and **not the original ABI**. The original command `_WriteData` and other longer functions remain incompletely reconstructed. Passing source compilation alone does not establish whole-program equivalence or hardware readiness.

## 2026-10-14 — command2leitz output writer recovered

New `recovery/classic_command_write_data.[ch]` records manufacturer `_WriteData` (original command Mach-O `0x100001a80..0x100001c14`), whose machine instructions were captured from the SHA-confirmed official macOS installer in [reference run 38479552118](https://github.com/an3k/leitz-icon-linux/actions/runs/38479552118). The independently isolated implementation preserves the original socket selection (URI contains `ipp` and socket != -1), one `send` without retry, optional debug-file `fwrite`, conditional stdout `fwrite`, and acceptance of positive short writes. The diagnostic `fprintf` failure text and original global ABI remain incomplete; the recovered wrapper is not a byte-identical original source file. A separate C smoke test validates the sink decisions without opening a network connection or modifying the existing command entry.

All GCC, Clang, native macOS and OEM-reference CI jobs for [run 38479996800](https://github.com/an3k/leitz-icon-linux/actions/runs/38479996800) passed. This validates compilation and the existing reference scenarios, **not the untested writer's original socket error diagnostics**. Do not report the whole original executable as reconstructed.

## 2026-10-14 — authentic macOS classic test-raster reconstruction

After checking the latest branch `sources` state, both archived manufacturer Mach-O binaries were extracted directly from the original uploaded macOS installer and SHA-256 verified: raster2printer `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`, command2leitz `6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee`.

Their previously missing dormant `_test_raster` functions were independently disassembled in full at `0x100003c60..0x100003e40` and `0x100004290..0x100004470`; both have the same observed control flow. Newly recovered source under `recovery/diagnostics/original_test_raster.c` preserves the genuine CUPS raster-open/header/one-row-per-read/close sequence, native source-line diagnostic and copy count, original zero-only pixel-read exit and absent extra allocation/short-read guards. The CUPS mock is strictly isolated to `fixture/` and not used in the macOS production build.

The isolated GCC/Clang offline tests and AddressSanitizer/UBSan test passed locally. A native macOS SDK compilation and CI run are configured; their outcomes must be checked after the commit. No change was made to normal raster output, either original manufacturer file, or any `enhancements/` source. The new code **does not complete** either original executable and has not been compared by directly invoking the dormant function inside the original Mach-O.

## 2026-10-14 — completed recovery of both dormant _PrintPageDebugInfo functions

Both authentic binaries were disassembled directly (same verified SHA-256 identities recorded above). Raster executable `0x100003e50..0x100004419` and command executable `0x100004480..0x100004a49` contain equivalent original `_PrintPageDebugInfo` functions. The seven actual OEM `fprintf` format strings, seven source line numbers (385–389, 391, 392), original source pathname basename, CUPS header field selection, floats and unbounded `%s` string output are reconstructed in `recovery/diagnostics/original_page_debug.c` without deliberate enhancements.

GCC and Clang have compiled the new source locally and its exact-text offline test passed. macOS SDK compilation is configured and must be confirmed from the completed Actions run. Neither the OEM executable nor the ordinary printer/command filter is modified; the helper is dormant. This completes one more individually verified native function, **not** the complete source-reconstruction project.

## Verified compile and reference job for the recovered diagnostic functions

At commit `867492daeb82a0e1579f18092003240d4671355d`, [Sources - Reconstruction Compile (run 38482664031)](https://github.com/an3k/leitz-icon-linux/actions/runs/38482664031) finished with **all three jobs successful**: GCC/Clang isolated tests, original-derived macOS C tests and genuine macOS CUPS SDK compilation of `original_test_raster.c` and `original_page_debug.c`, and the existing original-program reference scenarios. Local AddressSanitizer/UndefinedBehaviorSanitizer checks for both individual recovered diagnostic helpers also passed.

This confirms the reconstructed dormant helpers compile and emit the documented outputs in isolation; it does **not** prove that the original dormant OEM functions were directly executed against matching inputs, nor does it establish whole-program behavior equivalence. The active print or command entrypoints and original Linux/macOS manufacturer files were not modified.
