#!/usr/bin/env bash
# Install the released legacy driver in an existing CUPS installation.
# No queue, CUPS server, or user configuration is changed.
set -euo pipefail
if (($#)); then echo 'Usage: install.sh' >&2; exit 2; fi
root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
: "${DESTDIR:=}"
: "${CUPS_SERVERBIN:=/usr/lib/cups}"
[[ "$CUPS_SERVERBIN" == /* && "$CUPS_SERVERBIN" != *..* ]] || exit 2
[[ -z "$DESTDIR" || "$DESTDIR" == /* ]] || exit 2
if [[ -z "$DESTDIR" && "$(id -u)" != 0 ]]; then
  echo 'ERROR: Installing system files requires root' >&2; exit 1
fi
copy(){
  install -D -m "$1" -- "$root/$2" "$DESTDIR$3"
}
copy 0755 usr/lib/cups/filter/leitz-cli "$CUPS_SERVERBIN/filter/leitz-cli"
copy 0755 usr/lib/cups/filter/leitz-rastertoicon "$CUPS_SERVERBIN/filter/leitz-rastertoicon"
copy 0700 usr/lib/cups/backend/leitzusb "$CUPS_SERVERBIN/backend/leitzusb"
copy 0755 usr/local/bin/leitz-status /usr/local/bin/leitz-status
copy 0644 usr/share/ppd/LeitzIcon.ppd /usr/share/ppd/LeitzIcon.ppd
echo 'Driver installed. Add the Leitz Icon with the existing CUPS administration interface.'
