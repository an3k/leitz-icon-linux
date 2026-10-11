#!/usr/bin/env bash
# Install only the Leitz Icon CUPS driver into an existing CUPS installation.
# Never create a queue, change cupsd.conf, or restart the CUPS server.
set -euo pipefail

if (($# != 0)); then
  echo 'Usage: install.sh (optional environment: DESTDIR, CUPS_SERVERBIN)' >&2
  exit 2
fi
root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
: "${DESTDIR:=}"
: "${CUPS_SERVERBIN:=/usr/lib/cups}"
if [[ "$CUPS_SERVERBIN" != /* || "$CUPS_SERVERBIN" == *..* ||
      ( -n "$DESTDIR" && "$DESTDIR" != /* ) ]]; then
  echo 'ERROR: CUPS_SERVERBIN and DESTDIR must be safe absolute paths' >&2
  exit 2
fi
if [[ -z "$DESTDIR" && "$(id -u)" != 0 ]]; then
  echo 'ERROR: Installation into system directories requires root' >&2
  exit 1
fi
copy() {
  local mode="$1" input="$2" target="$3"
  [[ -f "$root/$input" ]] || { echo "ERROR: Package is incomplete: $input" >&2; return 1; }
  install -D -m "$mode" -- "$root/$input" "$DESTDIR$target"
}
filter="$CUPS_SERVERBIN/filter"
backend="$CUPS_SERVERBIN/backend"
copy 0755 usr/lib/cups/filter/leitz-cli "$filter/leitz-cli"
copy 0755 usr/lib/cups/filter/leitz-rastertoicon "$filter/leitz-rastertoicon"
copy 0755 usr/lib/cups/filter/leitz-command "$filter/leitz-command"
copy 0700 usr/lib/cups/backend/leitzusb "$backend/leitzusb"
copy 0644 usr/lib/leitz/protocols/leitz-current.so /usr/lib/leitz/protocols/leitz-current.so
copy 0644 usr/lib/leitz/protocols/leitz-native.so /usr/lib/leitz/protocols/leitz-native.so
copy 0644 usr/share/ppd/LeitzIcon.ppd /usr/share/ppd/LeitzIcon.ppd
copy 0755 usr/local/bin/leitz-status /usr/local/bin/leitz-status
copy 0700 usr/local/bin/leitz-probe /usr/local/bin/leitz-probe
echo 'Leitz Icon driver files installed. Add the printer using the existing CUPS administration interface.'
