# Enhancements and unverified experimental C reconstruction

This top-level directory stores project-authored C code which must **not** be mistaken for unmodified manufacturer sources or a verified 1:1 reconstruction.

`macos-5.1.465-classic/reconstructed/` contains the pre-existing C reconstruction fragments and supporting checks, moved without editing their contents from `sources/macos-5.1.465-classic/reconstructed/`. This preserves the previous implementation including any intentional improvements rather than merging them into the manufacturer reference.

The experimental C modules can be built in part with `make clean all check`. The full raster CUPS adapter requires CUPS development headers and has not been built in this environment. The original macOS Mach-O programs have not been used for complete output/behavior comparison.

At least two changes from the examined original routine are known: an added flush-failure rejection and an extended negative header-result callback contract. Do not assume the remaining experiments are exhaustive or faithful.

For true original source code, see `sources/linux-1.0.3/`. For the macOS classic reverse-engineering audit, see `sources/macos-5.1.465-classic/`.
