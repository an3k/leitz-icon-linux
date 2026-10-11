#!/usr/bin/env bash
# Build a stand-alone CUPS driver package; no server and no test code included.
set -euo pipefail
root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)
cd "$root"
make all cups probe ppd
mkdir -p dist
stage=$(mktemp -d)
trap 'rm -rf -- "$stage"' EXIT
install -D -m 0755 leitz-cli "$stage/usr/lib/cups/filter/leitz-cli"
install -D -m 0755 leitz-rastertoicon "$stage/usr/lib/cups/filter/leitz-rastertoicon"
install -D -m 0755 cups/leitz-command "$stage/usr/lib/cups/filter/leitz-command"
install -D -m 0700 cups/leitzusb "$stage/usr/lib/cups/backend/leitzusb"
install -D -m 0644 modules/leitz-current.so "$stage/usr/lib/leitz/protocols/leitz-current.so"
install -D -m 0644 modules/leitz-native.so "$stage/usr/lib/leitz/protocols/leitz-native.so"
install -D -m 0644 cups/LeitzIcon.ppd "$stage/usr/share/ppd/LeitzIcon.ppd"
install -D -m 0755 cups/leitz-status "$stage/usr/local/bin/leitz-status"
install -D -m 0700 leitz-probe "$stage/usr/local/bin/leitz-probe"
install -m 0755 install/install.sh "$stage/install.sh"
printf '%s\n' 'Leitz Icon CUPS driver package.' \
  'Run install.sh as root in an existing CUPS environment.' \
  'This does not add or modify printer queues or the CUPS server configuration.' \
  > "$stage/INSTALL.txt"
tar -czf dist/leitz-icon-driver-linux-amd64.tar.gz -C "$stage" .
echo 'Built dist/leitz-icon-driver-linux-amd64.tar.gz'
