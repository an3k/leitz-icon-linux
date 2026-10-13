# Leitz Icon — OEM sources and C reconstructions

This **sources** branch is for manufacturer originals, faithful C reconstruction work, and its independent verification. The own Linux/CUPS printer driver is developed in a different branch; it is intentionally **not duplicated here**.

## Directory layout

| Directory | Purpose |
| --- | --- |
| [`originals/`](originals/) | **Unmodified vendor files** only: Linux 1.0.3 C, PPD and installer; classic macOS 5.1.465 PPD |
| [`reconstructions/`](reconstructions/) | Separately reconstructed C code, including incomplete functions and standalone tests |
| [`enhancements/`](enhancements/) | Modified, optimized or experimental implementations; **not original-equivalent by default** |
| [`analysis/`](analysis/) | Symbol inventories, native disassembly findings, documented outstanding work |
| [`verification/`](verification/) | OEM-versus-reconstruction comparisons and deterministic CUPS test inputs |
| [`tooling/`](tooling/) | Non-OEM build helpers (including the Linux 1.0.3 Makefile) |
| [`.github/workflows/`](.github/workflows/) | Only workflows for reconstruction and authentic source examination |

The macOS naming convention uses a **single directory segment**, `macos-5.1.465-classic`, in every category. No nested `macos-5.1.465/classic` layout is used.

### Available platforms

- **Linux 1.0.3:** [Manufacturer C and PPD](originals/linux-1.0.3/), untouched. Build helper: [tooling/linux-1.0.3/Makefile](tooling/linux-1.0.3/Makefile); original-source details: [analysis/linux-1.0.3/README.md](analysis/linux-1.0.3/README.md).
- **macOS 5.1.465 classic:** [OEM PPD](originals/macos-5.1.465-classic/), [raster2printer reconstruction](reconstructions/macos-5.1.465-classic/raster2printer/), [command2leitz reconstruction](reconstructions/macos-5.1.465-classic/command2leitz/), [shared diagnostics](reconstructions/macos-5.1.465-classic/shared/diagnostics/), [audit/status](analysis/macos-5.1.465-classic/), [vendor comparisons](verification/macos-5.1.465-classic/).
- **macOS V3 / Windows:** directories will be introduced when their authenticated original material and reconstruction files actually exist; no empty placeholders.

## Source fidelity and status

The original classic macOS Mach-O binaries are **not committed here**. CI retrieves the original manufacturer installer, verifies the recorded checksums, and compares the reconstructed output for **specific inputs**. Compilable code and individual successful comparisons are **not** proof of complete equivalence to every original branch or to real hardware. Consult [the current classic reconstruction status](analysis/macos-5.1.465-classic/RECONSTRUCTION_STATUS.md).

Do not merge intentional improvements from `enhancements/` into OEM-faithful source without separate evidence; preserve the manufacturer's original behavior, including its documented limitations.
