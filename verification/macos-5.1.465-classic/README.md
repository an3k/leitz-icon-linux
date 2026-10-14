# Classic macOS original-versus-reconstructed runtime comparison

This directory contains *test input generation and comparison scripts*, not
manufacturer C and not printer functionality.

On a hosted Intel macOS runner, the reference workflow fetches the original
5.1.465 installer from Leitz's own service center, expands the installer and
checks **both** extracted Mach-O SHA-256 digests against the ones measured
from the original archive uploaded by the user. Neither executable is
committed to GitHub.

A deterministic two-page CUPS raster input is fed to the original
`raster2printer` and the newly authored classic stdout CUPS bridge with
identical PPD and options for OptimizationLevel 0/1/2/3.
The comparison checks exit status and the entire binary stdout.
It does not attach a printer or prove cutting, firmware status or network/PRN
side effects. *Only* an explicitly logged EXACT_OUTPUT=YES for a given mode
can count as runtime reference equality for that specific input.

The C code in `../../reconstructions/macos-5.1.465-classic/raster2printer/` is still incomplete as a 1:1 full native
macOS driver; differences must be treated as reconstruction defects,
not replaced with guesses or silent enhancements.


## Genuine command filter side-by-side execution

The original `command2leitz` and the independently reconstructed macOS
CUPS command entry are run as separate Intel macOS processes on the same
synthetic CUPS command file. `mock_backchannel.c` is an in-process macOS
dynamic interposer for `cupsBackChannelRead`; no real printer is contacted.
Cases cover `ReportStatus`, `GetUnitedStatus`, `GetFirmwareVersion`, an
unknown command, no backchannel response and an explicit read error.

The comparison records return codes, raw standard-output bytes, and
`STATE:/ATTR:` lines independently. It also verifies that the original
process actually invoked the interposer. A successful build or matching
synthetic output is NOT proof of equivalence for untested side effects,
device status values, or macOS file/notification paths.

## Whole-job original-derived C entry comparison (2026-10-14)

`compare_connected_with_vendor.sh` runs the newly compiled
`original_raster2printer_cups_candidate` and the verified original
manufacturer `raster2printer` as **separate** macOS processes with
identical PPD, device URI, two-page raster data and options. The
reference run [38561245944](https://github.com/an3k/leitz-icon-linux/actions/runs/38561245944)
reports four `OEM_CONNECTED_C_PARITY=YES` cases (mono/RGB24
and modes 2/3). These are exact stdout and exit-code comparisons,
not standalone unit tests. Other modes and printer communication
remain outside this proof.

## Exact joined original command entry comparison (2026-10-14)

The `compare_connected_command_with_vendor.sh` regression
compares **three complete process outputs** (stdout, stderr,
exit status) for SHA-256-verified vendor `command2leitz` against
the separately linked original-derived CUPS command candidate.
Both use the same independently validated local backchannel
interposition. [Run 38564165909](https://github.com/an3k/leitz-icon-linux/actions/runs/38564165909)
reports `OEM_COMMAND_CONNECTED_PARITY=YES` in all **13**
status and error-response cases, including native diagnostic
line endings and log-file errors. No printer is connected; these
specific scenarios are not proof of complete native binary
equivalence.

## Joined native original variable-length and PRN comparison (2026-10-14)

`compare_connected_extended_with_vendor.sh` checks complete byte output and exit status of the original Mach-O vs the joined native CUPS reconstruction: 33 and 127 CUPS raster rows, cutter OFF, modes 2/3, plus a separate original PRN transfer with full stdout and PRN-file comparison. All five cases passed in [run 38574138676](https://github.com/an3k/leitz-icon-linux/actions/runs/38574138676), and the results were maintained following `_FlushDataStreams` integration in [run 38574758345](https://github.com/an3k/leitz-icon-linux/actions/runs/38574758345). No real printer was involved.

## Exact native OEM raster stderr reference (2026-10-14)

`compare_connected_with_vendor.sh` now compares complete stderr as well as stdout and return status for each two-page monochrome/RGB24 mode-2/3 case. After correcting manufacturer packed-width diagnostics and the original `_ProcessPage` initial 0xff sentinel condition, [run 38593969771](https://github.com/an3k/leitz-icon-linux/actions/runs/38593969771) passed all four whole-process exact-matching cases, together with the existing variable-length, PRN and connected command comparisons. Transient disassembly-only CI logging was removed after its results were incorporated into the reconstructed C comments and analysis report. No hardware was used.

## Original joined raster startup/error verification (2026-10-14)

`compare_connected_startup_errors_with_vendor.sh` compares entire stdout, entire stderr, and exit codes from verified original macOS 5.1.465 `raster2printer` and its independently compiled original-derived native CUPS candidate for six deterministic cases: invalid argument counts 5/8; absent input file; absent PPD; zero-byte raster file; empty stdin. [Run 38605605549](https://github.com/an3k/leitz-icon-linux/actions/runs/38605605549) established **6/6 exact parity**. [Run 38606149312](https://github.com/an3k/leitz-icon-linux/actions/runs/38606149312) reconfirmed the same six plus all previous normal raster, variable-row, PRN and command cases after original status/URI symbols were linked. Real network/USB communication is not part of this proof.

## Native OEM mode-1 RGB24 whole-job comparison (2026-10-14)

`compare_mode1_rgb_with_vendor.sh` exercises the complete original-derived native macOS CUPS candidate and original manufacturer `raster2printer` with equal `rgbtailblank` raster inputs, genuine Bayer8x8 dithering, optimization mode 1, heights 33/80 raster rows and cutter ON/OFF. [CI 38611488870](https://github.com/an3k/leitz-icon-linux/actions/runs/38611488870) verified four exact stdout/stderr/exit matches. The last RGB row is white on purpose: the original mode-1 row copy reads four extra bytes after nonblank rows, so this input avoids undefined last-row behavior without changing the reconstructed OEM implementation. No hardware was connected.

## Native connected RGB24 halftone and local IPP routing (2026-10-14)

`compare_connected_halftones_with_vendor.sh` compares manufacturer Mach-O with the joined recovered OEM CUPS executable for all 13 original RGB24 quality matrices not previously exercised through the joined path, at 300/600 dpi, OEM modes 2/3. [Run 38623180688](https://github.com/an3k/leitz-icon-linux/actions/runs/38623180688) confirmed **52/52 exact stdout, stderr and exit matches**. The original Draft choice is a 1-bit mode, not an RGB24 table. No mode 0/1 memory-unsafe path is used here.

`compare_connected_ipp_loopback.py` independently launches the manufacturer and joined reconstruction with `DEVICE_URI=ipp://127.0.0.1/ipp/print`, fresh **127.0.0.1-only TCP/9100 listeners**, and byte-comparison of four independent results: TCP payload, stdout, complete stderr, exit. [Run 38623604108](https://github.com/an3k/leitz-icon-linux/actions/runs/38623604108) confirmed manufacturer and candidate both used **stdout rather than TCP** for this CUPS raster case (3,620 stdout, 0 TCP, 9,404 stderr bytes, exit 0), and reconfirmed 52/52 RGB24 quality matches. No real printer or external network endpoint was accessed; socket/IPP printing remains unverified.

## Defined manufacturer mode-0 outputs and multiple command requests (2026-10-14)

`compare_connected_mode0_defined_with_vendor.sh` executes the original macOS Mach-O and the joined CUPS raster reconstruction in mode 0 with mono/RGB, 33/80 raster rows, cutter ON/OFF. `compare_defined_bytes.py` identifies original two-page EOF boundaries using `ESC G ESC E` for cutter ON and `ESC G` for cutter OFF and excludes **exactly** four undefined original memory-read bytes before each boundary. All other stdout bytes, complete stderr and process exit must agree. [Run 38625963144](https://github.com/an3k/leitz-icon-linux/actions/runs/38625963144) confirmed **8/8 defined-output matches**, not full-byte equivalence of undefined manufacturer memory. The error in the former fixed-size footer recognizer was corrected, not the OEM printer encoder.

The already connected macOS `command2leitz` status main has six additional exact original comparisons for multi-command ordering, mixed case and early `ReportStatus` termination. [Run 38624982713](https://github.com/an3k/leitz-icon-linux/actions/runs/38624982713) passed **19/19** command scenarios (stdout, stderr, exit), with no source-code algorithm changes.
