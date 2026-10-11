# Leitz Icon — Linux / Unraid / CUPS

Native Linux CUPS driver for the Leitz Icon USB label printer.

**State:** Active development, not yet qualified for production use.
The default branch `main` is reserved for approved releases.

## Driver components

- `src/leitz_raster.c`: CUPS raster and media processing.
- `src/leitz_jobs.c`: document, page, and cutting coordination.
- `src/leitz_protocols.c/.h`: dynamic protocol loader and API.
- `src/leitz_protocol_current.c`: Current protocol encoder and validator.
- `src/leitz_protocol_native.c`: Native Linux protocol encoder and validator.
- `cups/leitzusb`: USB transfer and cartridge preflight.
- `media/media.json`: authoritative catalog of 86 media profiles.
- `build/generate_media.py`: generates PPD and media definitions.

## Protocol selection

The permanent CUPS `LeitzProtocol` option offers `Auto` (default),
`Current`, and `Native`. Auto prefers Current and falls back to
Native if Current is unavailable. Explicit selections never fall back.

Protocol modules are independent shared libraries:
`modules/leitz-current.so` and `modules/leitz-native.so`.
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

## Current protocol: original-Linux-derived raw-row option

The Original Linux 1.0.3 `Native` module is kept unchanged. The `Current`
module additionally supports `LeitzCurrentRaster=FullWidthRaw`, using
full-width packed 1-bit rows without RLE and a fresh `ESC D` width command
per nonblank line, as in the original Linux 1.0.3 optimization-1 mode,
but **retaining Current framing and cutting**.
The default `Optimized` mode remains byte-identical to the previous Current
implementation. The new mode is experimental and not hardware-qualified.
Choosing `Native` with `FullWidthRaw` is rejected, not silently ignored.

## Known limitations

The older Current implementation physically printed white 88 mm continuous
media at 300×300 and 300×600 dpi. The development Current validator now
rejects mixed-resolution pages within a single spool, mirroring the
document-level resolution invariant of the original Linux 1.0.3. This
does not change Current's existing wire output or the frozen Native
encoder. Hardware qualification of the modular build, Native output,
additional cassette types, and reliable mechanical completion/status
reporting is still pending. No released build is claimed.

## Installing into an existing CUPS server

Build a driver-only package (no CUPS server or Docker image):

```sh
bash install/package.sh
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
protocol module before opening USB. Only the validated maximum print
width is returned; the USB transport does not select, inspect or log
the protocol encoder. Invalid or incomplete results block the job.

A successful write is **not** a verified mechanical print or cut completion.
Uncertain transmission outcomes are never automatically retried.

### Original Linux optimization-2 principle in Current

`LeitzCurrentRaster=TrimmedRaw` forces uncompressed one-bit raster bytes
with leading and trailing white-byte margins removed, analogous to the
original Linux optimization-level-2 raster strategy. It retains Current
framing and cutting; it does not reproduce the vendor's entire protocol.
`Optimized` remains the byte-identical default, `FullWidthRaw` is unchanged,
and both optional modes are experimental and rejected by Native.
