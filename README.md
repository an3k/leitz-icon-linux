# Leitz Icon Linux — USB/CUPS driver

**Projektstand und offene Punkte:** [docs/PROJECT_STATUS.md](docs/PROJECT_STATUS.md). **Aktive Entwicklung:** `port/vendor-parity`; `main` ist der ältere v0.4.1-Vergleichsstand. Keine Hardwarefreigabe des aktuellen Entwicklungsbranches.

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

## USB readiness before printing

The development `leitzusb` backend now checks the installed cartridge with `ESC U`, then sends `ESC A` and requires the **measured ready status byte `0x03`** before transmitting any print bytes. A missing or different status blocks the job; no automatic retry is attempted. This is not a physical printing/cutting completion acknowledgment. The running TARS container is not modified.

## Shared driver architecture (development branch)

`leitz-rastertoicon` uses `leitz_cups_filter.c` for the **common**
CUPS/media/raster path and `leitz_job.c` for **one common**
page, label-length, cutter-policy and document controller. That controller
switches **only the wire encoder** between `src/leitz_protocol_current.c` (`Current`)
and `src/leitz_protocol_native_linux_103.c` (`NativeLinux103`). Both protocols now
implement `LabelCutter=ON/OFF` and multi-page documents through the
same job controller. Validation still checks the exact selected framing.
Each protocol is an independently selectable optional module; either can be omitted from a build.
the temporary alternate codec and tests are retained during comparison.

## Actual CUPS print protocol selection (development only)

The development-branch CUPS queue has `LeitzProtocol`: `Current` (**default; existing tested implementation**) or `NativeLinux103` (**original Linux 1.0.3 framing, experimental**). The same `leitz-rastertoicon` filter processes media, raster data and length selection, then encodes in the chosen protocol. The `leitzusb` backend independently validates the selected binary stream before USB and performs the same cartridge-width preflight. Both protocols support multiple labels at a **single print width and resolution per job**, with variable lengths on continuous media. The common controller and both spool validators reject page-width changes. The shared `LabelCutter=ON` option (default) cuts each label; `OFF` emits a final cut after the whole job **in either wire dialect**. The no-cut-between-pages path in `Current` is newly implemented and remains hardware-unverified. This functionality has not been tested on the actual printer. This is a single driver with two independently removable wire encoders, not two separate printer systems.

## Original Linux protocol validation (offline only)

The original Linux 1.0.3 raw-reference framing is checked independently with `leitz-cli validate-native file.bin`; the existing hardware-tested dialect remains `leitz-cli validate file.bin`. The original Linux reference-derived encoder leaves trailing blank rows unflushed; production `NativeLinux103` does the same. The experimental native-only flush in the common controller has been removed. Any resulting mechanical length difference needs physical verification. `make native-reference-test` includes both binary frame tests and rejection of native frames by the production validator. The development CUPS raster filter now selects the native dialect explicitly, and the USB backend validates its frames; physical verification is outstanding.

## Additional production-path functionality (development branch)

- The CUPS PPD now offers `ColorModel=Gray8` with selectable `LeitzDither=Bayer4` or `Threshold`. Both render 8-bit CUPS DeviceK/DeviceGray to the existing one-bit device language; the original `ColorModel=Black` route remains unchanged. The proprietary macOS algorithms are not claimed to be identical.
- `cups/leitz-command` supports CUPS maintenance command `ReportStatus`. It passes a tightly restricted internal status request to the exclusive `leitzusb` backend, which issues `ESC A` without printing or cutting. This is an **on-demand idle status query**, not a completion signal or automatic recovery mechanism. Build, PPD and offline filter tests have passed; hardware validation is still outstanding.
- The backend and raster validators reject malformed or blank printer commands before USB transfer. The verified `Current` encoder bytes are preserved.

## Manual device status probe (development branch only)

`port/vendor-parity` includes the optional `leitz-probe`. It sends **one** operator-approved `ESC A`, `ESC V`, or `ESC P` query to an idle printer and displays **both the original binary reply as hexadecimal and its conservative decoding** (status mappings, printable ASCII and fields, or an unverified printed-line candidate), without files or bind mounts. Linux kernel `usblp` prevents concurrent opens of `/dev/usb/lpN`; no extra `flock` paths are needed. This does **not** establish mechanical idle or verified print completion. See [controlled status probe](docs/CONTROLLED_STATUS_CAPTURE.md). Offline tests: `make probe-test`. The new probe code has not yet been tested on TARS.

## Optional wire modules and single-source paper catalog

The production wire implementations are **exactly two independent C files**:
`src/leitz_protocol_current.c` and `src/leitz_protocol_native_linux_103.c`.
Every page goes through the shared `src/leitz_job.c` and `src/leitz_cups_filter.c`;
each codec independently owns its binary encoder and validator. No codec-specific
media rules are applied by the shared controller. The common `src/leitz_protocol.h`
API is implemented by either module and dispatched by `src/leitz_protocol_registry.c`.

Deleting either source file from a source distribution removes that codec from
the resulting driver binary. `make all cups ppd` generates a filtered
`dist/LeitzIcon.ppd`: when both are present, CUPS offers both choices; with
only one source, only the installed protocol is selectable and is the default.
The Docker image installs this **generated** PPD, not the unfiltered template.
`make protocol-isolation-test` builds and checks both reduced variants offline.

The authoritative maintained format catalog is `media.json`. The synchronized
C header (`src/leitz_media.h`) and CUPS template (`cups/LeitzIcon.ppd`)
remain separate representations of the same 86 profiles, verified by
`make media-check`. They are not yet generated from a single source.
