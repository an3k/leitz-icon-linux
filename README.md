# Frozen CUPS base for Unraid

This branch is **not** the Leitz printer driver. It owns the independent,
read-only upstream CUPS snapshot and the two package operations.

- `upgrade`: compare the upstream `olbat/cupsd:latest` linux/amd64 image
  with the pinned source; only when changed, download once, archive the
  original Docker image, extract its layers once, and save **both**
  immutable files as versioned GitHub Release assets. Update
  `cups/source.lock` to select that immutable version. Previous source
  archives are never overwritten.
- `build`: consume only the selected saved source archive; combine the
  independently checked-out `main` release driver with a temporary CUPS
  build environment. Produce a separate loadable Unraid Docker image.
  This operation must never pull a moving CUPS source tag or edit
  an upstream source archive.

The original CUPS image's startup command, user setup, installed drivers,
and printer configuration are preserved. There is no custom Leitz
container entrypoint; add the new printer using the usual CUPS interface.

`driver` provides separately installable development driver packages.
`tests` verifies installation into this same CUPS base in an isolated
CI workspace, without changing the original files.

**Initial source state:** No snapshot exists until `upgrade` succeeds.
A Docker image is a collection of filesystem layers and an image
configuration, rather than an unpackable Unraid application executable.
The two source archive files retain both the original image and the
unpacked layer representation.
