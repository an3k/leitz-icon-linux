# Classic macOS 5.1.465 — active raster source audit (2026-10-13)

## Evidence / method

Manufacturer file: `raster2printer`, **Mach-O x86_64, 84,252 bytes**, SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13` from the user's original installer. The source of each address below is direct inspection of the executable's disassembly; the reconstructed C is **not** vendor source. No code was copied from the Linux or Windows drivers to assert macOS behavior.

Within the manufacturer `_ProcessPage` function (`0x100007560..0x1000089a6`), inspection of the 955 disassembly lines found **48 conditional jump instructions**. That count is *not* 48 user features, not a line coverage metric, and not a denominator for a percentage of reconstructed functions. Some conditions are loop guards, data-length decisions, or compiler-emitted checks. The four optimization modes are represented separately in C; no complete direct binary execution equivalence has been established.

## Original branch inventory and status

| Original address | Active decision / action | Reconstructed C | Evidence status |
|---|---|---|---|
| `0x100007633` | Modes 0/1 preamble `ESC D <stride> SYN` | `mac_classic_row_preamble` | Opcode and condition established |
| `0x1000078dc` | Mode 0 bypasses white-row scan | `mac_classic_row_encode` | Original conditional established |
| `0x10000889b` | Mode 0 sends raw row including white | Same | Original raw output path identified |
| `0x1000078eb..0x100007972` | Find leading nonwhite byte; defer fully white rows | `mac_classic_row_encode` | Original loop and 16-bit white-row counter identified |
| `0x100007972..0x100007a74` | Emit delayed white-row advance `ESC f` before nonempty row | Same | Command bytes identified; *end-of-page* white rows not flushed |
| `0x100007a74..0x100007e69` | Mode 3 byte-edge handling and raw raster | Same | Original path identified, with special trailing-whitespace condition at `0x100007ab4` |
| `0x100007b56` | Mode 3 left-offset command only if changed | Same | Prior offset local to page |
| `0x100007c71` | Mode 3 width command only if descriptor cache changes | Same | Cache persists between page calls |
| `0x100007e6e` | Mode 1 plain full-width row after white-line handling | Same | Raw transfer identified |
| `0x100007f32..0x100008882` | Mode 2 edge trimming, byte-run recognition, bit-run fallback | Same | Byte-level fragments reconstructed, not entire compiler control flow |
| `0x100007fc2` | Mode 2 emits left-offset command | Same | Command dispatch identified |
| `0x10000815f..0x10000824d` | Cached width and explicit reprint-first-row exception | Same | Reprint flag is separate from normal print path |
| `0x1000085ba..0x10000887d` | Compressed `0x17` if shorter, else raw `0x16` | Same | 13,200 independent *reconstructed-model* offline comparisons previously passed; not a running Mach-O comparison |
| `0x1000077a6` | Host raster progress every 32 rows | `mac_classic_progress` | Page encoding progress, not hardware acknowledgment |
| `0x100008945` | Check SIGTERM flag after every row, including last row | `mac_classic_job` | Implemented after-last-row cancellation guard |

## Newly recovered active output boundary

Original `_EndPage` at `0x1000089b0..0x100008ab1` sends `ESC G` and optionally `ESC E`. At **`0x100008a8d`**, *after* `_WriteData`, it calls `_FlushDataStreams`.

The latter at `0x1000015f0..0x10000162c` calls `fflush(stdout)` **unconditionally**, and `fflush(__fd_dump)` when a debug/PRN sink is present. The preceding reconstruction delayed its stdout flush until the end of the entire job, incorrectly moving the manufacturer's per-page output boundary.

**Correction:** `mac_classic_job_io` now has an optional `flush(write_ctx)` callback. The job coordinator invokes it exactly once after each emitted page-end command and treats a flush failure as an error **before** counting that page completed. The CUPS adapter flushes the actual selected sink: `stdout` in direct mode, anonymous temporary PRN storage in deferred mode. The original flush routine ignores the `fflush` result; rejecting a failed flush is intentional safety hardening.

## New header-read failure handling

Original `_ReadPage` uses `cupsRasterReadHeader2` and a zero/nonzero return. The reconstruction's portable callback has more expressive return values. Previously it interpreted *any* nonzero value, including negative, as a page; that could conceal a failed header read when the caller implements negative errors.

**Correction:** callback `read_header` now defines `1=page`, `0=EOF`, `negative=error`. Negative results abort before pixel allocation. This is a deliberately enhanced callback contract; the existing CUPS adapter still receives 0 for both true EOF and failures reported as zero by `cupsRasterReadHeader2`. A further CUPS-native error distinction remains unverified.

## Checked locally

- `make clean all check` with GCC and Clang, C11, `-Wall -Wextra -Werror`: passed.
- Two-page flush order and failed-first/failed-second-page flush cases: passed.
- Short pixel read, negative header callback with otherwise valid header, SIGTERM during last row: passed.
- Clang AddressSanitizer and UndefinedBehaviorSanitizer on the job and general smoke programs: passed.
- `make mac_classic_cups_filter` was **attempted but did not build**, because this environment does not contain the real `cups/cups.h` (nor the CUPS development SDK). A successful native CUPS raster-filter build is **not** claimed. The separate command executable links to the available `libcups.so.2` runtime.
- The original Mach-O binary has **not** been executed against identical input. No hardware printing or cut behavior has been verified.

## Remaining source-reconstruction gaps

1. Full original `_ProcessPage` assembly-level path correspondence, including all branches and original per-descriptor state and error exits, has not been proven.
2. Full integration of all original macOS PPD option handling, networking, PRN/debug side effects, and original raster memory layout remains incomplete.
3. Entire raster CUPS adapter must be built with genuine CUPS 2.x SDK; macOS-native build remains outstanding.
4. Exact-byte comparisons using original macOS executable and real device print/cut verification remain outstanding.

**Do not infer progress percentages from the four optimization modes or the 48 machine-code jumps.** Distinguish recovered code, locally tested reconstructed behavior, genuine vendor parity, and real printer confirmation.

## 2026-10-15 — mode-3 descriptor width state recovered from original 0x24 field

**Original manufacturer evidence:** [Bounded native audit 38778100810](https://github.com/an3k/leitz-icon-linux/actions/runs/38778100810) checks the original 5.1.465 raster2printer Mach-O hash `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13` before `otool` disassembly. In OEM `_ProcessPage` mode 3, `0x100007c71` loads the descriptor pointer, `0x100007c78` reads `0x24(%rdx)` as 32 bits, and `0x100007c7b` compares that stored number with the current trimmed row width. If different, `0x100007ca2` takes width AL, `0x100007ca5` zero-extends the byte into EAX, and `0x100007cb0` writes that 32-bit value into the same descriptor `+0x24` **before** `0x100007d6d` calls `_WriteData` for the 3-byte ESC D command. Thus the original holds only the lower 8 width bits after issuing ESC D, even though the comparison uses the full cropped row width. Native reprint path 0x100006135 also resets the SAME descriptor +0x24 field. This is distinct from mode 2, which reads and writes its separately recovered process-static width symbol.

**Source recovery:** [Commit 1d3ddcdf](https://github.com/an3k/leitz-icon-linux/commit/1d3ddcdfb95424c181a79edd7eddb778c74d7408) replaces opaque bytes +0x24..+0x27 with named `uint32_t mode3_previous_width` and keeps unknown bytes +0x28..+0x2f opaque. The name describes observed use; it is not a claim about the original C++ member spelling. The mode-3 `ProcessPage` wrapper now synchronizes its existing row encoder's *working* cache from this actual descriptor before processing each page. When emitting ESC D in mode 3, it writes `(uint8_t)new_width` into descriptor +0x24 **before** invoking the original WriteData callback, then mirrors that stored value back to the working cache for subsequent rows. This retains all previously validated byte encoders, WriteData calls and paper/cutter behavior, and preserves mode-2's independent static symbol. The dormant OEM reprint helper now resets the exposed field rather than writing through an opaque byte offset.

**Focused source checks:** Existing one-byte-width mode-3 page cases remain unchanged. Additional source fixture cases demonstrate that a deliberately stale working cache cannot override descriptor +0x24, and that a 260-byte-wide two-row mode-3 page emits ESC D 0x04 on **both** rows: the OEM compares 260 to previously stored 4. This is an original disassembly-derived result; no claim is made that an arbitrary physical printer supports an unbounded-width cassette. Final executable/vendor parity validation is tracked in the subsequent full CI record.
