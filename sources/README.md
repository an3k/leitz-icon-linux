# Manufacturer originals and C reconstructions

This directory is dedicated to **manufacturer source material**, independent of the project's own raster encoders.

| Manufacturer variant | Original archive | Confirmed files on this branch | Reconstruction status |
|---|---|---|---|
| Linux 1.0.3 | `LeitzIconDriversForLinux1.0.3.zip` | Original C sources, headers, PPD, installation script | Complete manufacturer C source, byte-exact |
| macOS 5.1.465, classic (PPD 1.2.190) | `LeitzIconSoftware-Installer_5.1.465.dmg.zip` | Original PPD; Mach-O symbol inventories | **C reconstruction incomplete**; original binaries disassembled offline |
| macOS 5.1.465, V3 | Same macOS archive | Not yet processed | Pending separate phase |
| Windows 5.0.3879.0 | `LeitzIconLabelSetup.zip` | Not yet processed | Pending separate phase |

The original executable `raster2printer` inside the Linux 1.0.3 archive is Mach-O, despite being inside a Linux driver archive; the Linux C code must be built to produce a Linux executable.

**Do not represent assembly, pseudocode, or incomplete functional equivalents as fully compilable C source.** A C reconstruction is accepted only after recovering the functions/data, making it buildable, and separately checking behavior.

See the version-specific README files for original checksums and limitations. Source extraction does not imply that any feature has been integrated into the development driver's raster encoder.
