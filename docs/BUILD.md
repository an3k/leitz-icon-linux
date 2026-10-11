# Building the v0.4.1 driver from main

This document applies to the older experimental `main` driver only.
The active `driver` branch contains a newer, independently developed
protocol implementation and uses different media metadata and build tools.

## Requirements

A Linux environment with GCC, make, the CUPS development headers,
Python 3 and standard archive utilities. The commands below do not
download, install, configure or replace a CUPS server.

## Verify and compile

```sh
python3 build/check_media.py
make clean all cups
cupstestppd cups/LeitzIcon.ppd
```

The media check is read-only. It compares exactly **76** v0.4.1
profile identifiers and geometries across `media/media.json`,
`src/leitz_media.h` and `cups/LeitzIcon.ppd`. It also checks the
CUPS custom-page declaration. It must not rewrite the original media
display strings, because the JSON catalog does not include all PPD
labels.

## Package

```sh
bash install/package.sh
```

The output is `dist/leitz-icon-driver-main-linux-amd64.tar.gz`.
It contains only the legacy CUPS filter, validation CLI, USB backend,
status utility, PPD and `install.sh`. There is **no bundled CUPS
server** and no separate protocol modules in this version.

This is a compiled experimental driver, not a validated production
release. Build checks do not confirm print movement or cutting.
