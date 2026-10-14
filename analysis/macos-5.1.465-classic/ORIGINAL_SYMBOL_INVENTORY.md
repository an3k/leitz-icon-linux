# macOS Classic 5.1.465 — complete exported/native symbol inventory

Evidence: original 5.1.465 installer downloaded on Intel macOS,
both executables verified with the uploaded original SHA-256 hashes.
Actual macOS `nm -nm` listings: [GitHub Actions 38467472696](https://github.com/an3k/leitz-icon-linux/actions/runs/38467472696).
The symbol list has **31 entries for raster2printer** and **20 entries for command2leitz** including binary header, start stub, and local compiler/library helpers. These are counts of symbols, **not completeness percentages**.

## raster2printer — 31 symbol entries

| OEM symbol | Original macOS offset | Current reconstructed scope |
|---|---|---|
| `start` | 0x100001410 | Not recovered independently |
| `_WriteData` | 0x100001450 | Recovered manufacturer 3-register + 0x10a8-byte **by-value** descriptor call surface, proven at OEM `_CheckPrinterStatus` 0x100009759–0x100009792 (socket in `%edx`, `rep movsq` of 0x215 qwords, `callq _WriteData`). Linked into joined native CUPS candidate and delegates to existing verified `classic_write_data_core`, exact output parity CI 38651986542. Other private C++ ABI/incomplete error paths remain open |
| `_FlushDataStreams` | 0x1000015f0 | Recovered stdout/optional PRN flush and connected via named `FlushDataStreams` entry to joined CUPS output lifecycle; exact native C++ global-state ABI remains open |
| `_is_bit_set` | 0x100001630 | Recovered OEM 32-bit bit-test helper exposed as named `is_bit_set` entry in joined raster executable; encoder still uses independently verified identical bit arithmetic |
| `_OpenDebugFile` | 0x100001660 | Recovered OEM PRN/debug file opening now linked through named `OpenDebugFile` and used by the joined raster job |
| `_OpenLogFile` | 0x100001910 | Original log helper already linked as named `OpenLogFile` into joined macOS raster main; original private global layout remains a C reconstruction |
| `_SendDebugFileToPrinter` | 0x100001b40 | Original byte-at-a-time PRN transfer connected via named `SendDebugFileToPrinter` to joined raster job; exact OEM PRN output externally compared |
| `_CloseDebugFile` | 0x100001e10 | Recovered original PRN close/clear, connected via named `CloseDebugFile` to joined job cleanup |
| `_CloseLogFile` | 0x100001e50 | Original log closure already linked as named `CloseLogFile` into joined macOS raster main |
| `_WriteDebugBuffer` | 0x100001e90 | Original conditional `fwrite` PRN action linked through named `WriteDebugBuffer` into joined `_WriteData` sink path |
| `_output_cups_info` | 0x100001ef0 | Manufacturer Linux C body and macOS diagnostics reconstructed earlier; now linked as dormant `output_cups_info` into the native joined raster candidate; original invalid-PPD behavior retained |
| `_test_raster` | 0x100003c60 | Earlier assembly-derived `test_raster` function now linked into joined native raster CUPS candidate; stays dormant in normal printing |
| `_PrintPageDebugInfo` | 0x100003e50 | Seven original diagnostic calls reconstructed earlier; `PrintPageDebugInfo` now linked as dormant native raster executable symbol, without adding new calls to normal printing |
| `__Z14signal_handleri` | 0x100004420 | Original diagnostic and termination state unchanged; exact manufacturer C++ Mach-O function symbol now exported on macOS using the original int handler calling convention, linked to `_main` and validated by native `nm`; broader C++ globals/ABI still not proven |
| `_main` | 0x100004460 | OEM-derived C entry now linked into `original_raster2printer_cups_candidate` with actual Apple CUPS SDK; OEM symbol ABI and every original error/global case remain incomplete |
| `_raster2printer` | 0x1000045a0 | Original-connected CUPS raster lifecycle, four ProcessPage modes and RGB24 DitherPage; active inline read now uses assembly-confirmed **unsigned quotient / signed remainder** at 0x1000057eb/0x100005800, sharing the recovered _ReadPage formula. Defined manufacturer output parity CI 38653552241; original mode-0/1 overreads and full native ABI remain incomplete |
| `_StartDocument` | 0x100006890 | Complete defined output/write-count sequence reconstructed in `original_raster_document_lifecycle.[ch]`; reusable original command bytes retained |
| `_DitherPage` | 0x100006dd0 | Full defined RGB24 path including OEM descriptor and diagnostics reconstructed in `original_dither_page.[ch]`, reusing the existing halftone loop; now also wired through `original_raster_dither_bridge.[ch]` into the joined source-level raster job (GCC, Clang and macOS CI verified); original global/undefined-case ABI pending |
| `_StartPage` | 0x100007380 | Native call sequence including 11-byte `_WriteData`, job counter, original 16-bit height arithmetic and descriptor by-value transfer recovered in `original_raster_page_lifecycle.[ch]` |
| `_ProcessPage` | 0x100007560 | Normal-mode loop reconstructions now exist separately for 0/1 and 2/3, reusing existing OEM byte encoders. For modes 2/3, command boundary writes, progress, job counter, termination and persistent row-width caches are implemented in `original_raster_process_page_modes23.[ch]`; full native ABI and undefined behaviors still incomplete; normal mode-2/3 dispatch is connected |
| `_EndPage` | 0x1000089b0 | Native 2/4-byte `_WriteData`, counter and unconditional `_FlushDataStreams` recovered in `original_raster_page_lifecycle.[ch]` |
| `_EndDocument` | 0x100008ac0 | Complete defined ESC E / conditional flush, ESC @ ESC Z, job size, CUPS close and stream-pointer reset recovered in `original_raster_document_lifecycle.[ch]` |
| `_ReadPage` | 0x100008d20 | Original field-offset descriptor and all established defined read branches; corrected **unsigned `divl` quotient + signed `idivl` remainder** packed-stride calculation at 0x100008e68/0x100008e82, with high-bit 32-bit regression cases (CI 38653552241). C++ ABI and whole-executable integration incomplete |
| `_CheckPrinterStatus` | 0x100009190 | Original descriptor-**pointer** entry confirmed at 0x1000091a9 (`%rdi` stored as pointer, subsequently dereferenced at +0x50 and passed unchanged to `_ResolveDeviceURI`); linked CUPS entry retains the already correct pointer and recovered 0x10a8-byte media-reset copy. Live socket/URI global rereads verified offline. Undefined errors, original C++ internals and hardware unverified (CI 38651132555) |
| `_ResolveDeviceURI` | 0x100009a30 | Previously reconstructed original CUPS URI parsing and TCP/9100 resolver now linked via named `ResolveDeviceURI` to the dormant status entry in the macOS raster candidate; OEM error/undefined paths preserved, network and hardware behavior not tested |

Other symbol entries: `__mh_execute_header` at 0, `__ZL7strrchrUa9enable_ifIXLb1EEEPKci` at 0x100004570, `__ZL6strstrUa9enable_ifIXLb1EEEPcPKc` at 0x100008cf0, `__ZL12_OSSwapInt16t` at 0x100009fe0, `__ZL16__libcpp_strrchrPKci` at 0x10000a010, and `__ZL15__libcpp_strstrPKcS0_` at 0x10000a040. These five local helper bodies are now independently recovered in `reconstructions/macos-5.1.465-classic/raster2printer/original_raster_private_helpers.c`; their original C++ mangled ABI remains separate from the C test interface.

## command2leitz — 20 symbol entries

| OEM symbol | Original macOS offset | Current reconstructed scope |
|---|---|---|
| `start` | 0x100000ad0 | Not recovered independently |
| `_main` | 0x100000b10 | Full defined control-flow reconstruction in `original_command_main_flow.c`; exact original diagnostic sites; isolated test/candidate, original ABI and boundary cases not yet certified |
| `_GetPrinterStatus` | 0x1000011d0 | Recovered ESC A query and source-line 123 diagnostic; now exposed as native executable function name `GetPrinterStatus`, called by reconstructed OEM main, seven defined responses original-verified; native ABI differs |
| `_GetUnitedStatus` | 0x1000012a0 | Recovered ESC U query and source-line 132 diagnostic; now exposed as `GetUnitedStatus` and called from OEM main, three responses verified |
| `_GetFirmvareVersion` | 0x100001370 | Original spelling preserved; now exposed as `GetFirmvareVersion` and called by OEM main, three ESC V response cases original-verified |
| `_GetStatus` | 0x100001440 | Original status/error body now exposed as named `GetStatus`, shared by the three active OEM query functions and original-verified across 13 defined cases; C callback ABI and unchecked failures incomplete |
| `_EndOfJob` | 0x100001a30 | Original ESC Z and flush now exposed as `EndOfJob`, invoked by `GetStatus` and unit-tested independently |
| `_WriteData` | 0x100001a80 | OEM signed send/stdout/dump behavior reused from `classic_command_write_data.c`; the joined macOS command candidate now exposes original-style data/size/socket plus 0x10a8 working-descriptor-by-value C entry with the OEM's +0x50 scheme access, and native `__fd_stdout`/`__fd_dump` FILE globals (CI 38650073372). Full C++ descriptor ABI still unproven; dormant in ordinary command processing |
| `_FlushDataStreams` | 0x100001c20 | OEM-defined stdout / optional global `__fd_dump` flush behavior now exposed through the original symbol-level C call surface, reusing the prior recovered utility; linked into command candidate |
| `_is_bit_set` | 0x100001c60 | Recovered 32-bit masked x86 shift and bit test; now exposed with OEM-compatible global function signature in command helper module |
| `_OpenDebugFile` | 0x100001c90 | Isolated original C body recovered in `command2leitz/original_command_file_helpers.c`; active integration pending |
| `_OpenLogFile` | 0x100001f40 | Isolated original C body recovered with unchanged 1024-byte path buffer and 0755/0644 operations |
| `_SendDebugFileToPrinter` | 0x100002170 | Original byte-by-byte fgetc/fputc transfer and diagnostics recovered; tested on isolated PRN fixture |
| `_CloseDebugFile` | 0x100002440 | Original global `__fd_dump`: conditional fclose then reset to NULL, using existing recovered implementation; unit-tested |
| `_CloseLogFile` | 0x100002480 | Original global `__fd_log`: conditional fclose then reset to NULL, using existing recovered implementation; unit-tested |
| `_WriteDebugBuffer` | 0x1000024c0 | Original non-null tests and `fwrite(data,length,1,stream)` recovered |
| `_output_cups_info` | 0x100002520 | Existing recovered `output_cups_info` from shared diagnostics now linked into native command candidate; dormant in normal status path |
| `_test_raster` | 0x100004290 | Shared reconstructed `test_raster` linked into native command candidate; remains dormant and not invoked by normal command file |
| `_PrintPageDebugInfo` | 0x100004480 | Seven original OEM diagnostic calls retained in shared reconstructed `PrintPageDebugInfo`, now linked into native command candidate; stays dormant |

Other entry: `__mh_execute_header` at zero.

## Newly examined original CheckPrinterStatus entry

Original disassembly `0x1000091c8..0x100009243` directly establishes
an early branch that calls `_ResolveDeviceURI` and immediately exits the
status-check routine when `ipp` is present and the socket is -1,
or when the termination flag is set and `ipp` is present.
With an already initialized socket and no termination, the routine proceeds
to the status-exchange body instead.
This decision is now isolated in `reconstructions/macos-5.1.465-classic/raster2printer/original_status_gate.[ch]`.

**No part of this inventory implies complete decompilation.** The native
original has already been executed to verify many printing cases, but full
control/error/file/network side-effect equivalence for both programs remains
outstanding. The genuine compiler-generated original C source does not exist
in the vendor installer, and original source wording cannot be recovered
verbatim from these executables.

**Native OEM whole-output comparison (2026-10-14):**
[run 38561245944](https://github.com/an3k/leitz-icon-linux/actions/runs/38561245944)
matched four original two-page outputs exactly (mono/RGB, mode 2/3)
through the newly linked native CUPS entry. This is stronger evidence
than an isolated-function test but does not establish binary/native
ABI equivalence or hardware print/cut confirmation.

**Command `_main` + `_GetStatus` joined native comparison (2026-10-14):**
[run 38564165909](https://github.com/an3k/leitz-icon-linux/actions/runs/38564165909)
confirmed byte-identical **stdout AND entire stderr** and identical
exit status for 13 synthetic OEM backchannel replies. The comparison
confirms the interposer acted in both executable processes. This
does not prove all original command paths or ABI equivalents.

**Connected manufacturer stderr equality (2026-10-14):** [CI run 38593969771](https://github.com/an3k/leitz-icon-linux/actions/runs/38593969771) verifies full stdout, entire stderr and process exit for the joined macOS original-derived CUPS filter in four full mono/RGB jobs, modes 2/3. OEM `_DitherPage` packed-width diagnostics and the `_ProcessPage` source-line 860 initial 0xff sentinel were recovered using actual manufacturer execution plus native disassembly. This establishes tested observable output equality, not full native ABI equivalence or live hardware behavior.

**Defined mode-1 RGB24 native manufacturer parity (2026-10-14):** [run 38611488870](https://github.com/an3k/leitz-icon-linux/actions/runs/38611488870) verified all four two-page RGB24 `_DitherPage`→`_ProcessPage` mode-1 combinations: heights 33 and 80 raster rows, cutter ON/OFF; **full stdout and stderr bytes plus exit codes match original**. All cases use a trailing white raster row, avoiding the original undefined last-row +4-byte overread. This does not prove mode 0, unsafe mode 1 inputs or full native ABI equivalence.

**Joined macOS RGB24 completeness and local IPP routing (2026-10-14):** [run 38623604108](https://github.com/an3k/leitz-icon-linux/actions/runs/38623604108) confirms 52/52 exact two-page manufacturer RGB24 outputs, diagnostics and exits across the 13 remaining non-Draft PPD quality tables, two resolutions and OEM modes 2/3. The joined `_raster2printer` also matches the manufacturer under local `ipp://127.0.0.1/ipp/print`: 3,620 stdout bytes, 9,404 stderr bytes, exit 0, zero TCP bytes. These checks confirm the original **stdout routing** in that specific IPP environment; they do not verify active TCP/IPP status or native ABI behavior.

**Mode-0 OEM defined-byte evidence (2026-10-14):** [CI 38625963144](https://github.com/an3k/leitz-icon-linux/actions/runs/38625963144) compares the joined original-derived CUPS executable and verified OEM for mono/RGB24, 33/80 CUPS rows and cutter ON/OFF (8/8). All defined stdout bytes, entire stderr and exit codes match. The original mode-0 source reads 4 undefined extra bytes per page, so exactly 8 bytes of each two-page job are not compared. This is not a native ABI or physical-device certification.

**macOS file-private symbol naming (2026-10-14):** Existing reconstructed global `classic_original_socket` and `classic_original_resolved_uri` now bind to manufacturer Mach-O `__ZL4sock` and `__ZL12resolved_uri` on Apple, as proven by direct original disassembly. Five previously recovered internal C++ utility wrappers have the same manufacturer symbol labels on native builds: `__ZL7strrchrUa9enable_ifIXLb1EEEPKci`, `__ZL6strstrUa9enable_ifIXLb1EEEPcPKc`, `__ZL12_OSSwapInt16t`, `__ZL16__libcpp_strrchrPKci` and `__ZL15__libcpp_strstrPKcS0_`. CI checks native `nm`; source-level symbol parity alone is not full C++ binary ABI parity.

**Original raster global state recovery (2026-10-14):** [CI run 38638374211](https://github.com/an3k/leitz-icon-linux/actions/runs/38638374211) verifies the native joined CUPS raster candidate now declares and uses the original 32-bit `__ZL8job_size` (one shared _main/status/page counter), `__fd_stdout` (actual stdout-or-null FILE pointer), `__fd_dump` and `__fd_log` (existing PRN/log helper stream pointers). The manufacturer x86-64 disassembly confirms these labels and _WriteData's FILE pointer gate. Original algorithm bodies and CUPS stream bytes are unchanged in all defined parity cases. These recovered native symbol names and selected state transitions do not establish complete C++ calling ABI or exact private memory layout.
