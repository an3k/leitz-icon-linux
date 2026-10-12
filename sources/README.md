# Manufacturer sources and C reconstructions

This directory stores **manufacturer source material**, not the project's own raster encoders.
Sources are separated by original platform and driver variant.

| Variant | Input archive | State | Contents |
|---|---|---|---|
| Linux 1.0.3 | `LeitzIconDriversForLinux1.0.3.zip` | Original C source present, byte-exact | `linux-1.0.3/` |
| macOS 5.1.465, classic | `LeitzIconSoftware-Installer_5.1.465.dmg.zip` | Mach-O disassembled; **complete C reconstruction not available** | `macos-5.1.465-classic/` |
| macOS 5.1.465, V3 | Same macOS archive | Not started in this task | To be examined after explicit user instruction |
| Windows 5.0.3879.0 | `LeitzIconLabelSetup.zip` | Not started in this task | To be examined after explicit user instruction |

Do not label symbol inventories, assembly listings, generic reimplementations, or pseudocode as **fully reconstructed compilable C**. A full reconstruction requires all functions, data, constants, dependencies, error paths, CUPS integration, and demonstration of successful compilation. Behavioral equivalence must be checked separately.

The Linux vendor C files and PPD here were copied without modification from the uploaded archive. Their original git object hashes have been checked against the extracted files.

This branch is independent of the development driver. Source archiving does not imply that any function has been integrated into the production raster encoder.

See each variant directory for detailed evidence and build prerequisites.
