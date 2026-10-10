# Leitz Icon Linux — USB/CUPS driver

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

## Repository plan

1. Preserve and review the experimentally developed driver sources.
2. Add reproducible source, CUPS PPD and Docker build checks through GitHub Actions.
3. Make downloadable releases only after checks pass and actual printer behavior is verified.

Do not run experimental builds against production print queues without reviewing their status.

## Scope and privacy

Only independently developed source code, build metadata and suitable test fixtures belong in this public repository. No proprietary vendor driver binaries, original proprietary GPD files, personal labels, packet captures containing personal data, or secrets.

## Licensing

**No license has been selected yet.** Public access to the repository does not imply permission to copy, modify, or redistribute its contents beyond applicable law.
