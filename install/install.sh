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
# Reconfigure only queues actually using our USB backend. Installing a newer
# PPD into /usr/share/ppd alone does not update /etc/cups/ppd/<queue>.ppd.
# Staged package verification (DESTDIR) and offline image builds do not touch
# printer queues. lpadmin is the supported CUPS queue update operation.
if [[ -z "$DESTDIR" ]] && command -v lpstat >/dev/null 2>&1 &&
   command -v lpadmin >/dev/null 2>&1 &&
   lpstat -r 2>/dev/null | grep -qx 'scheduler is running'; then
  while IFS= read -r queue; do
    [[ -n "$queue" ]] || continue
    lpadmin -p "$queue" -P /usr/share/ppd/LeitzIcon.ppd
    echo "Updated active CUPS PPD for Leitz queue: $queue"
  done < <(LC_ALL=C lpstat -v | sed -n 's/^device for \([^:]*\): leitzusb:.*/\1/p')
fi
echo 'Driver installed. Add the Leitz Icon with the existing CUPS administration interface.'
