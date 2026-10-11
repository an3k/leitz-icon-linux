# Leitz Icon Linux – Debian Bookworm CUPS for Unraid

Branch **cups** keeps the original Debian CUPS source package files and exactly one selected, fully extracted Debian source revision. The approved Leitz driver is obtained from **main** only when building.

## Original and unpacked source

- **sources/original/**: the four unmodified source package files from Debian for every archived version, each with its own filename followed by .sha256. Original files shared between Debian package revisions are stored only once.
- **sources/unpacked/cups-<full-debian-version>/**: the only active source revision, extracted using dpkg-source with Debian patches applied. The full Debian package revision is part of the directory name.

## Separate operations

1. **Upgrade**: Run build/upgrade.sh. Download and verify the latest Debian Bookworm source when necessary. Recover from locally archived originals when the active extracted directory is missing. Extract and validate a new revision *before* deleting the previous extracted revision. Preserve all verified original archives.
2. **Restore**: Run build/restore.sh followed by the exact archived version to select an older revision, without network access.
3. **Build**: Run build/build.sh after checking out the approved main branch into release/. Compiles CUPS from the only selected local source, installs the self-built Debian packages, adds the Leitz driver, validates the result, and exports a container image. Building never updates sources.

GitHub Actions can be triggered by editing and committing requests/upgrade.trigger, requests/restore.trigger (set version=...), or requests/build.trigger on the cups branch. The corresponding workflows also provide manual dispatch when available.

At most three unsuccessful download/verification attempts are allowed in a rolling twelve-hour window. Failure timestamps, when needed, are tracked in requests/.upgrade-failures. Successfully verified originals remain archived even if extraction fails.

CI compilation and container validation do not replace hardware printing, media, or cutter verification.
