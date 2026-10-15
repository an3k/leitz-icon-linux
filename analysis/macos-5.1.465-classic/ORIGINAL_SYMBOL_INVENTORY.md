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
| `_OpenDebugFile` | 0x100001660 | Original debug file opening reused unchanged in joined CUPS candidate. Native global `_dbgprn_filename` (1024-byte writable Mach-O common data at 0x1000114d0) is now restored and its bytes are synchronized with the already recovered helper's analysis-local pathname; original FILE pointer `__fd_dump` retained. Verified native symbol and full existing printer-output regressions CI 38744080583. No real PRN device test |
| `_OpenLogFile` | 0x100001910 | Original log opening reused unchanged from recovered C helper. The OEM `_log_filename` 1024-byte writable common array at 0x1000118d0 now exists in joined CUPS binary; its bytes are passed to/from the established helper-local pathname representation, with `__fd_log` preserved. Native symbol inventory and comparison CI 38744080583; actual C++ private state still incomplete |
| `_SendDebugFileToPrinter` | 0x100001b40 | OEM two-path epilogue 0x100001e05 `xorl %eax,%eax` / 0x100001e0c `retq` confirms a defined integer return of zero. The original-named macOS CUPS candidate now exposes `int SendDebugFileToPrinter(void)` rather than void, reuses the same original bytewise PRN helper, and returns its value; native signature compile probe CI 38743584470. The OEM global 1024-byte `_dbgprn_filename` is also reinstated at its data symbol location (CI 38744080583). Full C++ internals remain open |
| `_CloseDebugFile` | 0x100001e10 | Recovered original PRN close/clear, connected via named `CloseDebugFile` to joined job cleanup |
| `_CloseLogFile` | 0x100001e50 | Original log closure already linked as named `CloseLogFile` into joined macOS raster main |
| `_WriteDebugBuffer` | 0x100001e90 | Original conditional `fwrite` PRN action linked through named `WriteDebugBuffer` into joined `_WriteData` sink path |
| `_output_cups_info` | 0x100001ef0 | Manufacturer Linux C body and macOS diagnostics reconstructed earlier; now linked as dormant `output_cups_info` into the native joined raster candidate; original invalid-PPD behavior retained |
| `_test_raster` | 0x100003c60 | Earlier assembly-derived `test_raster` function now linked into joined native raster CUPS candidate; stays dormant in normal printing |
| `_PrintPageDebugInfo` | 0x100003e50 | Seven original diagnostic calls reconstructed earlier; `PrintPageDebugInfo` now linked as dormant native raster executable symbol, without adding new calls to normal printing |
| `__Z14signal_handleri` | 0x100004420 | Original diagnostic and termination state unchanged; exact manufacturer C++ Mach-O function symbol now exported on macOS using the original int handler calling convention, linked to `_main` and validated by native `nm`; broader C++ globals/ABI still not proven |
| `_main` | 0x100004460 | OEM-derived C entry now linked into `original_raster2printer_cups_candidate` with actual Apple CUPS SDK; OEM symbol ABI and every original error/global case remain incomplete |
| `_raster2printer` | 0x1000045a0 | Original-connected CUPS raster lifecycle, page reader, four ProcessPage modes and RGB24 dither. Native loop flag inventory already established zero-only reprint state (CI 38546921933); the dormant original ProcessPage(...,1)→EndPage caller is now independently reconstructed in `original_raster_active_page_output.c`, with separate mono/RGB unit tests (CI 38736163664). It is **not activated** in the connected normal CUPS loop. Full private ABI, defined-vs-undefined original memory reads and real-device behavior remain incomplete |
| `_StartDocument` | 0x100006890 | Native 0x10a8-byte by-value descriptor at caller 0x100005463..0x10000548e and callee rbp+0x10 verified; original-named signature now delegates to existing recovered document algorithm (CI 38728912252) |
| `_DitherPage` | 0x100006dd0 | OEM four-register caller ABI at 0x100005f9e..0x100005fb9 recovered in the original-named C symbol; original dithering core reused via private callback adapter (CI 38728610360) |
| `_StartPage` | 0x100007380 | Original **one descriptor-pointer** entry verified at caller 0x100005fbe/0x100005fc5 and callee 0x100007399. OEM-named entry and 11-byte original page-start algorithm now joined via a separate private callback adapter (CI 38713102329) |
| `_ProcessPage` | 0x100007560 | Four-register native C ABI and all four main modes already reconstructed. **Mode 3 native descriptor width cache** confirmed by checksum-verified `otool` at 0x100007c71..0x100007cb0: compare descriptor +0x24 32-bit to cropped row width, store the zero-extended LOW WIDTH BYTE into 32-bit +0x24 **before** the original 3-byte ESC D `WriteData` at 0x100007d6d. Source-level descriptor field `mode3_previous_width` and exact copy timing now replace the synthetic-only mode-3 working cache, including width >255 behavior; source commit 1d3ddcdf, OEM audit 38778100810. Mode 2 independently uses its original static symbol (0x10000815f/0x10000821f). Private C++ ABI and exceptional network/memory paths remain incomplete |
| `_EndPage` | 0x1000089b0 | OEM **0x10a8-byte stack-by-value descriptor** call verified at 0x100006016..0x10000605e and callee 0x1000089bb; original named C entry uses by-value ABI and reuses 2/4-byte output/flush algorithm (CI 38713102329) |
| `_EndDocument` | 0x100008ac0 | OEM by-value 0x10a8-byte descriptor verified at caller 0x1000062eb..0x10000632d and callee rbp+0x10; recovered command/flush/CUPS-close body reused unchanged (CI 38728912252) |
| `_ReadPage` | 0x100008d20 | Original five-register pointer ABI and integer return recovered earlier; active inline raster reader remains distinct. The common 0x10a8-byte native descriptor now models the CUPS 64-byte media-name field separately at +0x1050..+0x108f and the eight opaque bytes at +0x1090 before the URI port +0x1098. Compile-time field offsets and manufacturer CUPS/raster comparisons pass CI 38741475902; private C++ source and undefined behaviors remain open |
| `_CheckPrinterStatus` | 0x100009190 | Original descriptor-**pointer** entry (`%rdi`, 0x1000091a9); return epilogue 0x1000099f2..0x100009a1d masks one byte with `andb $1` then zero-extends into EAX. OEM-named entry now returns `_Bool`, with the existing internal int-valued callback reconstruction and undefined-case marker kept separate (CI 38732042515). Live socket/URI globals retained; unverified native error branches and hardware still open |
| `_ResolveDeviceURI` | 0x100009a30 | Original RDI receives the shared full 0x10a8-byte raster descriptor; integer return, URI scheme +0x50, hostname +0x450, resource +0x850, username +0xc50 and port +0x1098 are retained. The *earlier claim* of an 88-byte `cupsPageSizeName` array overlapping the URI port was not demonstrated by OEM instructions and is corrected: CUPS defines a 64-byte name field at +0x1050..+0x108f, with a separate port at +0x1098. Native and TCP loopback offline tests pass CI 38741475902; live network/error behavior unverified |

Other symbol entries: `__mh_execute_header` at 0, `__ZL7strrchrUa9enable_ifIXLb1EEEPKci` at 0x100004570, `__ZL6strstrUa9enable_ifIXLb1EEEPcPKc` at 0x100008cf0, `__ZL12_OSSwapInt16t` at 0x100009fe0, `__ZL16__libcpp_strrchrPKci` at 0x10000a010, and `__ZL15__libcpp_strstrPKcS0_` at 0x10000a040. These five local helper bodies are now independently recovered in `reconstructions/macos-5.1.465-classic/raster2printer/original_raster_private_helpers.c`; their original C++ mangled ABI remains separate from the C test interface.

## command2leitz — 20 symbol entries

| OEM symbol | Original macOS offset | Current reconstructed scope |
|---|---|---|
| `start` | 0x100000ad0 | Not recovered independently |
| `_main` | 0x100000b10 | Full defined control-flow reconstruction in `original_command_main_flow.c`; exact original diagnostic sites; isolated test/candidate, original ABI and boundary cases not yet certified |
| `_GetPrinterStatus` | 0x1000011d0 | Native `(bool, double)` register entry verified at 0x1000011d8/0x100001287; now exported from joined macOS candidate and delegates to the existing OEM-derived status body, with original stream parity (CI 38665905068). Full private runtime/ABI beyond known inputs remains unverified |
| `_GetUnitedStatus` | 0x1000012a0 | Native `(bool, double)` entry verified at 0x1000012a8/0x100001357, now called by original-derived command main and forwarding to the native `_GetStatus` entry; original output and errors unchanged (CI 38665905068) |
| `_GetFirmvareVersion` | 0x100001370 | OEM spelling preserved; native `(bool, double)` ABI verified at 0x100001378/0x100001427 and linked through `_GetStatus` without duplicating existing core (CI 38665905068) |
| `_GetStatus` | 0x100001440 | Manufacturer 4-argument native ABI `(char command, bool flag, const char *prefix, double timeout)` proven by `%dil`, `%sil`, `%rdx`, `%xmm0` at 0x10000144b..0x10000147c; exported in connected CUPS candidate, delegating to unchanged behavioral core. All 25 OEM command process comparisons retained (CI 38665905068) |
| `_EndOfJob` | 0x100001a30 | OEM one-`bool` call proven by 0x1000019e0..0x1000019eb (masked `%edi`), now exposed with original signature as a native entry, reusing previously recovered ESC Z / flush body. the joined native GetStatus success path now enters the actual _EndOfJob(bool) symbol via its existing shared callback core; no call on empty/error data, OEM output parity CI 38668597114 |
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

**OEM command prefix data objects reconstructed (2026-10-15):** Native manufacturer `nm` reports four **local 8-byte __DATA,__data pointers** at `0x1000061d0..0x1000061e8`: `_kPIPrinterStatusCommandPrefix`, `_kPIUnitedStatusCommandPrefix`, `_kPIFirmawareVersionCommandPrefix` (original typo), `_kPILeitzIconDomain`. Recovered callback and joined native command path now use shared file-local pointer objects with precisely those Apple assembler labels; they previously used arrays/literals. [CI 38702024563](https://github.com/an3k/leitz-icon-linux/actions/runs/38702024563) checks their native object labels, unit tests and complete manufacturer command output parity. This establishes original source-level data object identity and textual values, not exact OEM relocations or machine code.

## Original 5.1.465 native protocol-byte and error-stream data symbols (2026-10-15)

These global data objects are **in addition** to the native function-symbol
inventory above. Original addresses and sections come directly from the two
checked-in SHA-256-verified Mach-O `*.symbols.txt` listings. The three
read-only selector byte values derive from original Leitz Linux 1.0.3
`raster_c_functions.c` for ESC/SYN and the original 5.1.465 mode-2
raster disassembly and byte output for compressed selector 0x17.
The source-level object types are reconstructions, not recovered C++
type-declaration text.

| Original binary | Symbol | Manufacturer address | Recovered state |
|---|---|---|---|
| raster2printer | `_ESC` | `0x10000a438` | 1-byte read-only `0x1b` in `original_raster_private_helpers.c` |
| raster2printer | `_SYN` | `0x10000a439` | 1-byte read-only `0x16` |
| raster2printer | `_SYN_COMPRESSED_RASTER` | `0x10000a43a` | 1-byte read-only `0x17`, original `_ProcessPage` uses symbol at `0x1000085ef` |
| raster2printer | `__fd_err` | `0x1000114b0` | 8-byte zero-initialized writable native `FILE *` global, not conflated with `stderr` |
| command2leitz | `_ESC` | `0x100004d68` | 1-byte read-only `0x1b` in `original_command_file_helpers.c` |
| command2leitz | `_SYN` | `0x100004d69` | 1-byte read-only `0x16` |
| command2leitz | `_SYN_COMPRESSED_RASTER` | `0x100004d6a` | 1-byte read-only `0x17` |
| command2leitz | `__fd_err` | `0x100006210` | 8-byte zero-initialized native `FILE *` global; no new stderr routing inferred |

[Code commits 2a03c08b](https://github.com/an3k/leitz-icon-linux/commit/2a03c08bd15dfd4f66a09c887210681d958e80a4)
and [9f28594e](https://github.com/an3k/leitz-icon-linux/commit/9f28594ec34b0266e3fee607325afd7284f6a284)
restore these objects independently in each reconstructed executable;
unit tests assert all three protocol byte values, and native macOS
`nm` checks require every original data-symbol name in **both**
binaries. The OEM encoding, global stream operation and CUPS callback
algorithms are **not** rewritten. [CI 38774571374](https://github.com/an3k/leitz-icon-linux/actions/runs/38774571374)
passed all three jobs, the pre-existing 27 command and 90 connected
raster/PRN/IPP parity-success records, with zero reported parity failures.

A matching original data symbol and tested value are not evidence that
all original C++ translation-unit lifetime, initialization, errors or
hardware behavior have been reconstructed.

## Restored original 13 writable native halftone table symbols (2026-10-15)

The manufacturer 5.1.465 `raster2printer.symbols.txt` contains these
**13 separate original __DATA,__data 32-bit writable halftone table objects**.
The exact numeric array contents and all 13 original virtual locations were
**already** inventoried in `raster2printer_matrices_recovered.c`, each with
its previously authenticated SHA-256. No matrix values or dither algorithms
were reimplemented. The missing part was native object identity:
previously each array had only a synthetic `matrix_*` name and was declared
`const`, which on Apple ordinarily places the object in read-only data.
[Code commit 77b0a36b](https://github.com/an3k/leitz-icon-linux/commit/77b0a36ba0721184ecc517ad00207c63f61867fc)
restores exactly the OEM labels via Apple assembler aliases and preserves
the original writable allocation class. [Macro-scope fix 41670086](https://github.com/an3k/leitz-icon-linux/commit/4167008604db9209a420b2026eba9af88cad9191)
corrected a header-only compilation error before final verification.

| OEM table | Exact original symbol | Manufacturer VA | int32 elements |
|---|---|---|---:|
| `supercell` | `__ZL9supercell` | `0x10000c240` | 64 |
| `bayer4x4` | `__ZL8bayer4x4` | `0x10000c340` | 16 |
| `bayer8x8` | `__ZL8bayer8x8` | `0x10000c380` | 64 |
| `CUPSthreshold` | `__ZL13CUPSthreshold` | `0x10000c480` | 4096 |
| `orderedSuperCell` | `__ZL16orderedSuperCell` | `0x100010480` | 324 |
| `reorderedBayer3` | `__ZL15reorderedBayer3` | `0x100010990` | 144 |
| `supercell1` | `__ZL10supercell1` | `0x100010bd0` | 36 |
| `reorderedBayer` | `__ZL14reorderedBayer` | `0x100010c60` | 144 |
| `bayer14x14` | `__ZL10bayer14x14` | `0x100010f10` | 196 |
| `clustered8x8` | `__ZL12clustered8x8` | `0x100011220` | 64 |
| `dispersed8x8` | `__ZL12dispersed8x8` | `0x100011320` | 64 |
| `nasik4x4` | `__ZL8nasik4x4` | `0x100011420` | 16 |
| `dithering3x3` | `__ZL12dithering3x3` | `0x100011460` | 9 |

This restores native data-object names and writable C storage; it does **not**
claim exact original file-local C++ linkage, original virtual addresses in
the new executable, dynamic-loader relocations, or original source syntax.
The 13 tables' existing contents and 15 PPD quality selectors retain
the previous verified behavior. Final [CI 38780500037](https://github.com/an3k/leitz-icon-linux/actions/runs/38780500037)
passed Linux GCC/Clang, native macOS CUPS build and original-symbol checks,
and authenticated manufacturer executable output comparisons. The latter
still reports 27/27 command and 90 connected raster/PRN/IPP success lines,
zero `PARITY=NO`. No new hardware or original-private-branch parity claimed.
