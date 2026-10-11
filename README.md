# Leitz Icon — Linux / Unraid / CUPS

Native Linux CUPS driver for the Leitz Icon USB label printer.

**State:** Active development, not yet qualified for production use.
The default branch `main` is reserved for approved releases.

## Driver components

- `driver/src/leitz_raster.c`: CUPS raster and media processing.
- `driver/src/leitz_jobs.c`: document, page, and cutting coordination.
- `driver/src/leitz_protocols.c/.h`: dynamic protocol loader and API.
- `driver/src/leitz_protocol_current.c`: Current protocol encoder and validator.
- `driver/src/leitz_protocol_native.c`: Native Linux protocol encoder and validator.
- `driver/cups/leitzusb`: USB transfer and cartridge preflight.
- `driver/media/media.json`: authoritative catalog of 86 media profiles.
- `driver/build/generate_media.py`: generates PPD and media definitions.

## Protocol selection

The permanent CUPS `LeitzProtocol` option offers `Auto` (default),
`Current`, and `Native`. Auto prefers Current and falls back to
Native if Current is unavailable. Explicit selections never fall back.

Protocol modules are independent shared libraries:
`driver/modules/leitz-current.so` and `driver/modules/leitz-native.so`.
The installed loader uses `/usr/lib/leitz/protocols/`, or the optional
`LEITZ_PROTOCOL_DIR` path. Installed modules are discovered when a
new filter process starts. Adding/removing a library does not change the
CUPS PPD and needs no recompilation of the common driver.

## Build

On Linux with GCC, make, CUPS development headers, and Python 3 installed:

```sh
make clean all cups probe ppd
```

The 86-profile catalog is the sole source for generated media options.
Changes to `media.json` can be propagated using `make generate-media`.

## Known limitations

The older Current implementation physically printed white 88 mm continuous
media at 300×300 and 300×600 dpi. Hardware qualification of the modular
build, Native output, additional cassette types, and reliable mechanical
completion/status reporting is still pending. No released build is claimed.

## Installing into an existing CUPS server

Build a driver-only package (no CUPS server or Docker image):

```sh
bash driver/install/package.sh
```

This creates `dist/leitz-icon-driver-linux-amd64.tar.gz` containing an
`install.sh` and the production binaries, protocol libraries and PPD.
Extract it into a temporary directory and run `install.sh` as root **inside
the existing CUPS environment**, then register the Leitz printer through
normal CUPS administration. The installer never modifies other printers,
`cupsd.conf`, server credentials, or container configuration.

The optional frozen Unraid Community Applications CUPS integration is
maintained independently on the `cups` branch. The `driver` branch builds
without that integration.

## USB transfer result semantics

The backend validates the **whole** spooled document using an installed
protocol module before opening USB. The validator returns the exact maximum
print width and the protocol that accepted the spool. Both fields are
required; invalid or incomplete results block the job. The USB backend
logs this validated protocol instead of guessing from binary header offsets.

A successful write is **not** a verified mechanical print or cut completion.
Uncertain transmission outcomes are never automatically retried.
