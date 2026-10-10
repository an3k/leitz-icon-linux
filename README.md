# Leitz Icon Linux — USB/CUPS driver

An independent **Linux / CUPS driver** for the Leitz Icon USB label printer (USB VID:PID `289e:1000`).

> **Status: under development.** The v0.4.1 driver has passed specific physical print-and-cut tests on white continuous paper at both 300×600 and 300×300 dpi. This is **not** a fully validated release. The expanded 86-entry development PPD is offline-checked only and is not installed on TARS.

## Goals

- Direct USB printing on Linux via a native CUPS raster filter and a separate USB backend
- Cartridge detection using `ESC U` status queries, with media-width checks
- Choice of original Leitz media formats and user-defined label dimensions
- Print to continuous rolls with automatic cutting at the appropriate length
- 300×300 and 300×600 dpi where supported by the printer
- Self-contained driver: no dependence on a PDF-cropping tool or shipping-label processor

## Evidence and limitations

- **Physically tested with v0.2:** 88 mm continuous white paper, 300×600 dpi, a 170 mm calibration job, USB cartridge queries, CUPS printing from PDF/PostScript, and automatic cutting.
- **Physically tested with v0.4.1:** white continuous 88 mm paper at 300×300 and 300×600 dpi; fixed-length and content-dependent cutting; two-page jobs and PDF-through-CUPS printing. These tests do not establish coverage of all 86 media entries.
- **Open issues:** the 88 mm yellow-plastic cassette printed a shorter length than requested and a later test stopped partway through. Neither the cause nor the handling of all cassette SKUs, die-cut stock, and long-term reliability is established.
- **170 mm is only an example test length, not a fixed printer format.**
- Successful transmission to the USB printer does not prove physical completion.

## Source and continuous integration

The `main` branch contains the **experimental v0.4.1 development sources**, including:

- `src/`: protocol encoder, offline validator and CUPS raster filter
- `cups/`: USB backend, cartridge status, PPD with 76 profiles on `main` (86 A/B profiles on the development branch) and a standalone CUPS Dockerfile
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

## Development branch media profiles

`port/vendor-parity` keeps 43 sorted A profiles followed by 43 `Leitz`-prefixed B variants in the same PPD. The four differing original source geometries are preserved and must not be silently normalized. Use `make media-check` to check PPD, `media.json`, C header and original B labels without accessing the printer.

## Offline manufacturer-status reply decoding (development branch)

The existing `leitz-cli` can inspect **previously saved binary replies** with `leitz-cli decode-status legacy|v3|lines|version reply.bin` in `port/vendor-parity`. This reads ordinary files only; it never queries the printer. The original macOS status mappings and ESC P byte order inform these parsers but **hardware compatibility and print completion are not verified**. The development-only regression target is `make status-test`.

## Manual device status probe (development branch only)

`port/vendor-parity` includes the optional `leitz-probe`. It sends **one** operator-approved `ESC A`, `ESC V`, or `ESC P` query to an idle printer and displays the binary reply directly as hexadecimal, without files or bind mounts. Linux kernel `usblp` prevents concurrent opens of `/dev/usb/lpN`; no extra `flock` paths are needed. This does **not** establish mechanical idle or verified print completion. See [controlled status probe](docs/CONTROLLED_STATUS_CAPTURE.md). Offline tests: `make probe-test`. The new probe code has not yet been tested on TARS.
