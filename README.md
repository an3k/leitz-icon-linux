# Unraid CUPS source and package builder

This branch contains only the frozen upstream CUPS source and the tooling
for producing a complete container from the **approved** driver on `main`.
The development driver and its tests are maintained in independent branches.

## Local immutable source

`source/<SHA-256>/original/part-000...` is the unchanged upstream Docker
archive, stored as numbered 32 MiB files.
`source/<SHA-256>/unpacked/part-000...` is its once-extracted/repacked
Docker layer representation, also retained unchanged.
`source/<SHA-256>/SHA256SUMS` verifies both exact archives.
`source/source.lock` identifies the selected revision.

The numbered files are needed because GitHub limits individual file
sizes. They are committed as normal Git objects so the snapshot is visible
and locally available in the `cups` branch. Neither `build` nor any tests
change their bytes.

## Independent functions

- **Upgrade:** `build/upgrade.sh` checks upstream for a new digest. If
  missing, it archives the original and extracted source into a new
  version-specific directory and selects it in `source/source.lock`.
  Previously saved versions remain unchanged.
- **Build:** `build/build.sh` reads the selected unpacked source exclusively,
  verifies its checksum, and adds the current `main` release driver in a
  disposable build environment. It publishes a loadable Unraid Docker image.

To invoke through GitHub Actions, use the `cups` branch and edit either
`requests/upgrade.trigger` or `requests/build.trigger`. The actions are
independent. Upgrading CUPS is never implicit in building the image.

The image preserves the upstream CUPS configuration and entrypoint. The
Leitz Icon driver is not required for the base CUPS image to function.
For development installations, the separate `driver` branch builds a
driver-only archive which `tests` installs into a temporary CUPS environment.

**Verification:** Successful compilation and installation testing is not
a substitute for hardware print and cutter validation.
