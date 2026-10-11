#!/usr/bin/env bash
# Use the sole selected Debian source revision; no implicit upgrade.
set -euo pipefail
source "$(dirname "$0")/source-common.sh"
cd "$repo_root"
version=$(active_version) || { echo 'ERROR: Exactly one complete sources/unpacked/cups-<version> required.' >&2; exit 1; }
verify_originals "$version" || { echo "ERROR: Invalid originals for $version" >&2; exit 1; }
[[ -f release/Makefile && -f release/cups/LeitzIcon.ppd ]] || {
  echo 'ERROR: Missing approved main branch checkout in release/.' >&2; exit 1;
}
: "${RELEASE_SHA:?Missing main driver commit SHA}"
docker build --build-arg "CUPS_VERSION=$version" -f build/Dockerfile -t leitz-cups-complete:build .
docker run --rm --entrypoint /bin/bash leitz-cups-complete:build -euc '
  test -x /usr/sbin/cupsd
  test -x /usr/lib/cups/filter/leitz-rastertoicon
  test -x /usr/lib/cups/filter/leitz-cli
  test -x /usr/lib/cups/backend/leitzusb
  test -s /usr/share/ppd/LeitzIcon.ppd
  cupstestppd /usr/share/ppd/LeitzIcon.ppd
  test "$(dpkg-query -W -f="${Version}" cups)" = "$1"
' -- "$version"
mkdir -p dist
docker save leitz-cups-complete:build | gzip -1 > dist/leitz-icon-cups-unraid-image.tar.gz
printf '%s\n' "$version" > dist/cups-version.txt
printf '%s\n' "$RELEASE_SHA" > dist/leitz-main-commit.txt
(cd dist && sha256sum leitz-icon-cups-unraid-image.tar.gz > SHA256SUMS)
