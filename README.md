# Leitz Icon Linux – Debian Bookworm CUPS for Unraid

Branch **cups** keeps the original Debian CUPS source package files and exactly one selected, fully extracted Debian source revision. The CUPS build accepts **none**, **main** (default), or **dev**; each builds the same selected Debian source revision.

## Original and unpacked source

- **sources/original/**: the four unmodified source package files from Debian for every archived version, each with its own filename followed by .sha256. Original files shared between Debian package revisions are stored only once.
- **sources/unpacked/cups-<full-debian-version>/**: the only active source revision, extracted using dpkg-source with Debian patches applied. The full Debian package revision is part of the directory name.

## Separate operations

1. **Upgrade**: Run build/upgrade.sh. Download and verify the latest Debian Bookworm source when necessary. Recover from locally archived originals when the active extracted directory is missing. Extract and validate a new revision *before* deleting the previous extracted revision. Preserve all verified original archives.
2. **Restore**: Run build/restore.sh followed by the exact archived version to select an older revision, without network access.
3. **Build**: Run `bash build/build.sh [none|main|dev]` (default `main`). It compiles CUPS from the single selected Debian source revision, installs its self-built Debian packages, validates the image, and exports a loadable container archive. `none` adds **no** Leitz driver, PPD, backend, or printer queue. `main` adds the approved `main` driver; `dev` adds the development `driver` branch package. For `main`/`dev`, check out the selected branch into `release/` and export `RELEASE_SHA` to its exact commit SHA before invoking the local script. Building never updates the archived sources.

GitHub Actions can be triggered by editing and committing `requests/upgrade.trigger`, `requests/restore.trigger` (set `version=...`), or `requests/build.trigger` (set `driver=none|main|dev`; omitted = `main`) on the cups branch. Manual dispatch of **Build Debian CUPS** exposes the same three choices with `main` preselected. The `none` image is for **installing CUPS first, then separately installing the standalone driver archive**; do not use the integrated development image for this workflow. The CUPS container still includes its own password-protected CUPS startup script, which is not part of the upstream Debian source and does not precreate a Leitz queue in `none` mode.

At most three unsuccessful download/verification attempts are allowed in a rolling twelve-hour window. Failure timestamps, when needed, are tracked in requests/.upgrade-failures. Successfully verified originals remain archived even if extraction fails.

CI compilation and container validation do not replace hardware printing, media, or cutter verification.
