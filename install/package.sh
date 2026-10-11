#!/usr/bin/env bash
# Build a standalone archive of the approved main-branch driver.
set -euo pipefail
root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)
cd "$root"
python3 build/check_media.py
make clean all cups
mkdir -p dist
stage=$(mktemp -d)
trap 'rm -rf -- "$stage"' EXIT
install -D -m 0755 leitz-cli "$stage/usr/lib/cups/filter/leitz-cli"
install -D -m 0755 leitz-rastertoicon "$stage/usr/lib/cups/filter/leitz-rastertoicon"
install -D -m 0700 cups/leitzusb "$stage/usr/lib/cups/backend/leitzusb"
install -D -m 0755 cups/leitz-status "$stage/usr/local/bin/leitz-status"
install -D -m 0644 cups/LeitzIcon.ppd "$stage/usr/share/ppd/LeitzIcon.ppd"
install -m 0755 install/install.sh "$stage/install.sh"
tar -czf dist/leitz-icon-driver-main-linux-amd64.tar.gz -C "$stage" .
echo 'Created standalone main driver archive.'
