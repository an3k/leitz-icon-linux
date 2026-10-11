# Leitz Icon Linux — USB/CUPS driver

> **Branch notice:** `main` is the preserved, older experimental v0.4.1
> baseline, not a hardware-qualified release. Current standalone driver
> development takes place on [`driver`](https://github.com/an3k/leitz-icon-linux/tree/driver).
> The independent [`tests`](https://github.com/an3k/leitz-icon-linux/tree/tests)
> branch verifies it, while [`cups`](https://github.com/an3k/leitz-icon-linux/tree/cups)
> freezes the Community Applications CUPS base and builds a complete
> Unraid image from this `main` baseline. No release is approved yet.

An independent **Linux / CUPS driver** for the Leitz Icon USB label printer (USB VID:PID `289e:1000`).

> **Status: under development.** The older v0.2 prototype successfully printed and cut labels on a real device. The expanded v0.4 media/auto-cut implementation has not yet been physically validated. This repository does **not** currently provide a validated release.

## Goals

- Direct USB printing on Linux via a native CUPS raster filter and a separate USB backend
- Cartridge detection using `ESC U` status queries, with media-width checks
- Choice of original Leitz media formats and user-defined label dimensions
- Print to continuous rolls with automatic cutting at the appropriate length
- 300×300 and 300×600 dpi where supported by the printer
- Self-contained driver: no dependence on a PDF-cropping tool or shipping-label processor

## Evidence and limitations

- **Physically tested with v0.2:** 88 mm continuous white paper, 300×600 dpi, a 170 mm calibration job, USB cartridge queries, CUPS printing from PDF/PostScript, and automatic cutting.
- **Not established by that test:** functionality of all cassette SKUs and widths, die-cut media, 300×300 dpi, automatic cut at arbitrary content lengths, and long-term reliability.
- **170 mm is only an example test length, not a fixed printer format.**
- Successful transmission to the USB printer does not prove physical completion.

## Source and continuous integration

The `main` branch contains the **experimental v0.4.1 development sources**, including:

- `src/`: protocol encoder, offline validator and CUPS raster filter
- `cups/`: USB backend, cartridge status, PPD with 76 vendor media profiles and a standalone CUPS Dockerfile
- `media.json`: vendor-derived media dimensions, not evidence of physical support
- `tests/`: synthetic offline raster/validation tests (the original physical golden fixtures are not published)

[GitHub Actions](https://github.com/an3k/leitz-icon-linux/actions) compiles the sources, validates the PPD, runs synthetic tests and **builds a Docker image without access to real hardware**. A passed run is not a release or proof of printing success.

## Downloads and releases

- [Actions build results](https://github.com/an3k/leitz-icon-linux/actions): successful runs upload both a source snapshot (`leitz-icon-linux-source`) and an **experimental loadable Docker image** (`leitz-icon-cups-dev-image`). These are temporary build artifacts, not validated releases; GitHub may require login to download them.
- [Releases](https://github.com/an3k/leitz-icon-linux/releases): the tag-triggered `release.yml` workflow will build a **loadable Docker image archive**, source archive and SHA-256 checksums as a public prerelease when a `v*` tag is deliberately pushed.
- **No release tag has been created yet.** Do not assume a prebuilt Docker download already exists.
- [Unraid deployment notes](docs/UNRAID.md) describe intended fresh-container settings; deployment remains pending device testing.

Do not run experimental builds against production print queues without reviewing their status.

## Scope and privacy

Only independently developed source code, build metadata and suitable test fixtures belong in this public repository. No proprietary vendor driver binaries, original proprietary GPD files, personal labels, packet captures containing personal data, or secrets.

## Licensing

**No license has been selected yet.** Public access to the repository does not imply permission to copy, modify, or redistribute its contents beyond applicable law.
