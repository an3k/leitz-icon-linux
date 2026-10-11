# Leitz Icon Linux — standalone CUPS driver

This `main` branch preserves the older experimental v0.4.1 driver code.
It is **not** a hardware-qualified final release. The active standalone
development driver is on [`driver`](https://github.com/an3k/leitz-icon-linux/tree/driver);
verification is isolated in [`tests`](https://github.com/an3k/leitz-icon-linux/tree/tests).
The independent Debian Bookworm CUPS source and optional `none`/`main`/`dev`
container builds live on [`cups`](https://github.com/an3k/leitz-icon-linux/tree/cups).

## Contents

- `src/`: older CUPS raster filter, codec and CLI.
- `cups/`: PPD, USB backend and cartridge status command.
- `media/`: manufacturer-derived media catalog.
- `install/`: standalone driver archive builder and installer.
- `build/`: read-only validation of legacy media metadata.
- `docs/`: build and existing-CUPS/Unraid installation instructions.
- `Makefile`: compile the driver.

This branch deliberately does **not** contain integration tests or a
separate CUPS server image.

## Standalone installation

With compiler, CUPS development headers and Python 3 available, run
`bash install/package.sh` to create
`dist/leitz-icon-driver-main-linux-amd64.tar.gz`. Extract this archive
and run `install.sh` inside an existing Linux CUPS environment as root.
Then add the printer through normal CUPS administration. No existing
CUPS queues, credentials or configuration are replaced.

See [build instructions](docs/BUILD.md) and
[Unraid / existing-CUPS installation](docs/UNRAID.md).

## Limitations

The older prototype physically printed white 88 mm continuous paper;
not all media sizes, cassette variants, print completion semantics
or cutting behaviors have been verified with this baseline. USB data
delivery does not prove mechanical completion. No production release is
claimed.

No project-wide redistribution license has yet been selected.

## Workflow identity

`Driver - Main Build` creates the baseline standalone package. `Driver - Main
Release` publishes a versioned experimental prerelease for a `v*` tag.
Development driver builds and releases are separate workflows using `driver`
source and, for development releases, `dev-v*` tags.
