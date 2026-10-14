# macOS Classic 5.1.465 — complete exported/native symbol inventory

Evidence: original 5.1.465 installer downloaded on Intel macOS,
both executables verified with the uploaded original SHA-256 hashes.
Actual macOS `nm -nm` listings: [GitHub Actions 38467472696](https://github.com/an3k/leitz-icon-linux/actions/runs/38467472696).
The symbol list has **31 entries for raster2printer** and **20 entries for command2leitz** including binary header, start stub, and local compiler/library helpers. These are counts of symbols, **not completeness percentages**.

## raster2printer — 31 symbol entries

| OEM symbol | Original macOS offset | Current reconstructed scope |
|---|---|---|
| `start` | 0x100001410 | Not recovered independently |
| `_WriteData` | 0x100001450 | Partial, sink decisions |
| `_FlushDataStreams` | 0x1000015f0 | Isolated branch |
| `_is_bit_set` | 0x100001630 | Isolated recovered helper |
| `_OpenDebugFile` | 0x100001660 | Isolated PRN helper |
| `_OpenLogFile` | 0x100001910 | Isolated file helper |
| `_SendDebugFileToPrinter` | 0x100001b40 | Isolated PRN helper, one verified runtime |
| `_CloseDebugFile` | 0x100001e10 | Isolated helper |
| `_CloseLogFile` | 0x100001e50 | Isolated helper |
| `_WriteDebugBuffer` | 0x100001e90 | Isolated helper |
| `_output_cups_info` | 0x100001ef0 | Function body recovered and cross-checked with original Linux 1.0.3 source; see `reconstructions/macos-5.1.465-classic/shared/diagnostics/original_output_cups_info.c` |
| `_test_raster` | 0x100003c60 | Reconstructed from original assembly in `reconstructions/macos-5.1.465-classic/shared/diagnostics/original_test_raster.c`; dormant function, not called by normal printing |
| `_PrintPageDebugInfo` | 0x100003e50 | Seven original diagnostic calls recovered in `reconstructions/macos-5.1.465-classic/shared/diagnostics/original_page_debug.c`; not linked into the active filter |
| `__Z14signal_handleri` | 0x100004420 | Recovered original diagnostic + termination state in `original_raster_private_helpers.c`; C test-only function is not yet connected with active CUPS entry |
| `_main` | 0x100004460 | OEM-derived C entry now linked into `original_raster2printer_cups_candidate` with actual Apple CUPS SDK; OEM symbol ABI and every original error/global case remain incomplete |
| `_raster2printer` | 0x1000045a0 | Source-level joined C job executes original document/page lifecycle, the normal `_ProcessPage` modes 2/3 and the recovered RGB24 `_DitherPage` through its new callback bridge; mode 0/1 full-page integration still has an OEM four-byte overread, and full native CUPS/device ABI remains incomplete |
| `_StartDocument` | 0x100006890 | Complete defined output/write-count sequence reconstructed in `original_raster_document_lifecycle.[ch]`; reusable original command bytes retained |
| `_DitherPage` | 0x100006dd0 | Full defined RGB24 path including OEM descriptor and diagnostics reconstructed in `original_dither_page.[ch]`, reusing the existing halftone loop; now also wired through `original_raster_dither_bridge.[ch]` into the joined source-level raster job (GCC, Clang and macOS CI verified); original global/undefined-case ABI pending |
| `_StartPage` | 0x100007380 | Native call sequence including 11-byte `_WriteData`, job counter, original 16-bit height arithmetic and descriptor by-value transfer recovered in `original_raster_page_lifecycle.[ch]` |
| `_ProcessPage` | 0x100007560 | Normal-mode loop reconstructions now exist separately for 0/1 and 2/3, reusing existing OEM byte encoders. For modes 2/3, command boundary writes, progress, job counter, termination and persistent row-width caches are implemented in `original_raster_process_page_modes23.[ch]`; full native ABI and undefined behaviors still incomplete; normal mode-2/3 dispatch is connected |
| `_EndPage` | 0x1000089b0 | Native 2/4-byte `_WriteData`, counter and unconditional `_FlushDataStreams` recovered in `original_raster_page_lifecycle.[ch]` |
| `_EndDocument` | 0x100008ac0 | Complete defined ESC E / conditional flush, ESC @ ESC Z, job size, CUPS close and stream-pointer reset recovered in `original_raster_document_lifecycle.[ch]` |
| `_ReadPage` | 0x100008d20 | Complete defined branch recovery and original field-offset descriptor in `original_read_page.[ch]`; C++ ABI and whole-executable integration incomplete |
| `_CheckPrinterStatus` | 0x100009190 | Recovered normal status and retry logic plus original send/recv flags, complete 0x10a8-byte descriptor copy for reset and ignored WriteData result; isolated callback ABI, undefined socket failures and global integration remain incomplete |
| `_ResolveDeviceURI` | 0x100009a30 | Independently recovered native resolver, integration incomplete |

Other symbol entries: `__mh_execute_header` at 0, `__ZL7strrchrUa9enable_ifIXLb1EEEPKci` at 0x100004570, `__ZL6strstrUa9enable_ifIXLb1EEEPcPKc` at 0x100008cf0, `__ZL12_OSSwapInt16t` at 0x100009fe0, `__ZL16__libcpp_strrchrPKci` at 0x10000a010, and `__ZL15__libcpp_strstrPKcS0_` at 0x10000a040. These five local helper bodies are now independently recovered in `reconstructions/macos-5.1.465-classic/raster2printer/original_raster_private_helpers.c`; their original C++ mangled ABI remains separate from the C test interface.

## command2leitz — 20 symbol entries

| OEM symbol | Original macOS offset | Current reconstructed scope |
|---|---|---|
| `start` | 0x100000ad0 | Not recovered independently |
| `_main` | 0x100000b10 | Full defined control-flow reconstruction in `original_command_main_flow.c`; exact original diagnostic sites; isolated test/candidate, original ABI and boundary cases not yet certified |
| `_GetPrinterStatus` | 0x1000011d0 | Active query recovered |
| `_GetUnitedStatus` | 0x1000012a0 | Active query recovered |
| `_GetFirmvareVersion` | 0x100001370 | Active query recovered, OEM spelling retained |
| `_GetStatus` | 0x100001440 | Core status path recovered; side effects incomplete |
| `_EndOfJob` | 0x100001a30 | ESC Z recovered |
| `_WriteData` | 0x100001a80 | Original-specific implementation incomplete |
| `_FlushDataStreams` | 0x100001c20 | Original-specific implementation incomplete |
| `_is_bit_set` | 0x100001c60 | Helper not separately recovered for this executable |
| `_OpenDebugFile` | 0x100001c90 | Isolated original C body recovered in `command2leitz/original_command_file_helpers.c`; active integration pending |
| `_OpenLogFile` | 0x100001f40 | Isolated original C body recovered with unchanged 1024-byte path buffer and 0755/0644 operations |
| `_SendDebugFileToPrinter` | 0x100002170 | Original byte-by-byte fgetc/fputc transfer and diagnostics recovered; tested on isolated PRN fixture |
| `_CloseDebugFile` | 0x100002440 | Original-specific effects incomplete |
| `_CloseLogFile` | 0x100002480 | Original-specific effects incomplete |
| `_WriteDebugBuffer` | 0x1000024c0 | Original non-null tests and `fwrite(data,length,1,stream)` recovered |
| `_output_cups_info` | 0x100002520 | Same independently disassembled OEM function; shared recovered C source, not connected to live command entry |
| `_test_raster` | 0x100004290 | Same C behavior independently verified against original command binary disassembly; shared reconstructed source under `reconstructions/macos-5.1.465-classic/shared/diagnostics/` |
| `_PrintPageDebugInfo` | 0x100004480 | Same seven OEM calls independently established from this Mach-O; shared standalone reconstruction, dormant |

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
