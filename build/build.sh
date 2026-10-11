#!/usr/bin/env bash
# Build selected Debian CUPS from source; optional driver: none/main/dev.
set -euo pipefail
if (( $# > 1 )); then
  echo 'Usage: bash build/build.sh [none|main|dev] (default: main)' >&2
  exit 2
fi
mode=${1:-main}
case "$mode" in
  none|main|dev) ;;
  *) echo "ERROR: Unknown driver variant $mode; choose none/main/dev" >&2; exit 2 ;;
esac
source "$(dirname "$0")/source-common.sh"
cd "$repo_root"
version=$(active_version) || {
  echo 'ERROR: Exactly one complete sources/unpacked/cups-<version> required.' >&2
  exit 1
}
verify_originals "$version" || {
  echo "ERROR: Invalid archived originals for CUPS $version" >&2
  exit 1
}
driver_sha=none
target=cups-base
if [[ "$mode" != none ]]; then
  [[ -f release/Makefile && -f release/install/package.sh ]] || {
    echo "ERROR: Checkout branch $([[ "$mode" == dev ]] && echo driver || echo main) into release/" >&2
    exit 1
  }
  : "${RELEASE_SHA:?Missing selected driver commit SHA}"
  driver_sha=$RELEASE_SHA
  target=cups-with-driver
fi
docker build --target "$target" \
  --build-arg "CUPS_VERSION=$version" \
  --build-arg "DRIVER_MODE=$mode" \
  -f build/Dockerfile -t leitz-cups-complete:build .
docker run --rm --entrypoint /bin/bash \
  -e "CUPS_EXPECTED_VERSION=$version" -e "CUPS_EXPECTED_DRIVER=$mode" \
  leitz-cups-complete:build -euc '
    test -x /usr/sbin/cupsd
    test -x /usr/local/bin/cups-entrypoint
    test "$(dpkg-query -W cups | cut -f2)" = "$CUPS_EXPECTED_VERSION"
    test "$LEITZ_CUPS_DRIVER" = "$CUPS_EXPECTED_DRIVER"
    if [[ "$CUPS_EXPECTED_DRIVER" == none ]]; then
      test ! -e /usr/lib/cups/filter/leitz-cli
      test ! -e /usr/lib/cups/filter/leitz-rastertoicon
      test ! -e /usr/lib/cups/backend/leitzusb
      test ! -e /usr/share/ppd/LeitzIcon.ppd
      test ! -e /usr/lib/leitz/protocols/leitz-current.so
      test ! -e /usr/lib/leitz/protocols/leitz-native.so
    else
      test -x /usr/lib/cups/filter/leitz-cli
      test -x /usr/lib/cups/filter/leitz-rastertoicon
      test -x /usr/lib/cups/backend/leitzusb
      test -s /usr/share/ppd/LeitzIcon.ppd
      ppd_result=$(cupstestppd /usr/share/ppd/LeitzIcon.ppd 2>&1 || true)
      printf "%s\n" "$ppd_result"
      grep -Fq "/usr/share/ppd/LeitzIcon.ppd: PASS" <<< "$ppd_result"
      if [[ "$CUPS_EXPECTED_DRIVER" == dev ]]; then
        test -x /usr/lib/cups/filter/leitz-command
        test -s /usr/lib/leitz/protocols/leitz-current.so
        test -s /usr/lib/leitz/protocols/leitz-native.so
      else
        test ! -e /usr/lib/leitz/protocols/leitz-current.so
      fi
    fi
  '
mkdir -p dist
docker save leitz-cups-complete:build | gzip -1 > "dist/leitz-icon-cups-$mode-image.tar.gz"
printf '%s\n' "$version" > dist/cups-version.txt
printf '%s\n' "$mode" > dist/cups-driver-mode.txt
printf '%s\n' "$driver_sha" > dist/driver-commit.txt
(cd dist && sha256sum "leitz-icon-cups-$mode-image.tar.gz" > "leitz-icon-cups-$mode-image.tar.gz.sha256")
echo "PASS: CUPS $version variant $mode; driver commit $driver_sha"
