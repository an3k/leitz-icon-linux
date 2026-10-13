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

The C code in `../raster/` is still incomplete as a 1:1 full native
macOS driver; differences must be treated as reconstruction defects,
not replaced with guesses or silent enhancements.
