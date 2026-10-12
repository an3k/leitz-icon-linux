# Leitz Icon original Linux driver 1.0.3

The following files are **unmodified** copies of the uploaded manufacturer's archive:
- `raster2printer.c` (18,683 bytes; SHA-256 `bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0`)
- `raster_c_functions.c` (9,741 bytes; SHA-256 `977419445e283c5cf7a95d14a8d6b687330eb8256d25eefafa06d4e18fe3f74b`)
- `raster2printer.h` (579 bytes; SHA-256 `081b2c45d55a282bc79277c9731b4f4748024dedc569bc7aa195c4001e9102b9`)
- `raster_c_functions.h` (2,311 bytes; SHA-256 `150a95f22f4e8c82018bc8c5bc19d6ee749808ce6d52f53e064f82d8c55f815b`)
- `LeitzIcon.ppd` (29,861 bytes; SHA-256 `75a871a6ec5ef0af55e9a68b4c7128e86f093a2af04bce36ef7b7589e30989fb`)
- `install.sh` (1,000 bytes)

## Build (CUPS 2.x)

```sh
make
```

Original vendor command:
```sh
gcc -std=gnu99 -o raster2printer raster_c_functions.c raster2printer.c -lcups -lcupsimage
```

CUPS development headers/libraries are required. A successful compilation is **not yet verified in the present analysis environment**, which does not contain the required development headers. The supplied vendor `raster2printer` executable inside the Linux archive is a macOS Mach-O file, not a Linux ELF executable; compiling from C is required on Linux.

`install.sh` is preserved as evidence, not corrected here: it references `LeitzIcon.ppd.gz` although the archive contains the uncompressed `LeitzIcon.ppd`. Do not run the original installation script without reviewing its system modifications.

The original manufacturer's license/copyright headers remain in the files.
