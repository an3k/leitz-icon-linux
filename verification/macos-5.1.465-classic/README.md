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
