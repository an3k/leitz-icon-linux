#!/usr/bin/env bash
# Generic CUPS container: Debian CUPS with optional separately bundled Leitz driver.
set -Eeuo pipefail
umask 027

: "${CUPS_ADMIN_USER:=admin}"
: "${CUPS_SERVER_ALIAS:=}"

if [[ ! "$CUPS_ADMIN_USER" =~ ^[a-z_][a-z0-9_-]{0,30}$ ]]; then
  echo 'ERROR: Invalid CUPS_ADMIN_USER' >&2; exit 1
fi
if [[ -z "${CUPS_ADMIN_PASSWORD:-}" || ${#CUPS_ADMIN_PASSWORD} -lt 12 || "$CUPS_ADMIN_PASSWORD" == 'CHANGE_ME' ]]; then
  echo 'ERROR: Set CUPS_ADMIN_PASSWORD to a unique password (>=12 characters) via Unraid variable.' >&2
  exit 1
fi
if [[ -n "$CUPS_SERVER_ALIAS" && ! "$CUPS_SERVER_ALIAS" =~ ^[a-zA-Z0-9.\ _-]+$ ]]; then
  echo 'ERROR: Invalid CUPS_SERVER_ALIAS'; exit 1
fi

mkdir -p /etc/cups /var/spool/cups
if [[ ! -s /etc/cups/cupsd.conf ]]; then
  # Critical when Unraid bind-mounts an initially empty /etc/cups directory.
  cp -a /opt/cups-default/. /etc/cups/
  echo 'INFO: Initial CUPS configuration copied to persistent volume.'
fi
if ! grep -Eq '^(Port 631|Listen (\*:631|0\.0\.0\.0:631))$' /etc/cups/cupsd.conf; then
  sed -i -E 's/^Listen localhost:631$/Port 631/' /etc/cups/cupsd.conf
fi
if ! grep -Eq '^(Port 631|Listen (\*:631|0\.0\.0\.0:631))$' /etc/cups/cupsd.conf; then
  echo 'ERROR: CUPS must listen on port 631 for Docker port mapping.' >&2
  exit 1
fi

if ! id -u "$CUPS_ADMIN_USER" >/dev/null 2>&1; then
  useradd -m -G lpadmin -s /bin/sh "$CUPS_ADMIN_USER"
else
  usermod -aG lpadmin "$CUPS_ADMIN_USER"
fi
printf '%s:%s\n' "$CUPS_ADMIN_USER" "$CUPS_ADMIN_PASSWORD" | chpasswd
unset CUPS_ADMIN_PASSWORD

# Ensure the CUPS spool has suitable permissions on the persistent volume.
chown root:lp /var/spool/cups
chmod 0710 /var/spool/cups

/usr/sbin/cupsd -f &
cups_pid=$!
shutdown() {
  trap - TERM INT
  kill -TERM "$cups_pid" 2>/dev/null || true
  wait "$cups_pid" 2>/dev/null || true
}
trap shutdown TERM INT

ready=0
for _ in $(seq 1 30); do
  if lpstat -r >/dev/null 2>&1; then ready=1; break; fi
  if ! kill -0 "$cups_pid" 2>/dev/null; then break; fi
  sleep 1
done
if (( !ready )); then
  echo 'ERROR: CUPS did not start.' >&2
  shutdown
  exit 1
fi

# On first run, configure LAN printing and protected web administration.
# Do not overwrite a user's later cupsd.conf changes on container restarts.
if [[ ! -f /etc/cups/.cups-initialized ]]; then
  cupsctl --remote-any --remote-admin --share-printers WebInterface=Yes
  if [[ -n "$CUPS_SERVER_ALIAS" ]]; then
    printf '\nServerAlias %s\n' "$CUPS_SERVER_ALIAS" >> /etc/cups/cupsd.conf
    kill -HUP "$cups_pid" || true
  fi
  touch /etc/cups/.leitz-initialized
fi

# The 'none' variant stays a pure CUPS image, without Leitz queues.
# Preserve the historical automatic first-queue setup only for integrated
# main/dev variants. On 'none' the driver is installed separately by the user.
if [[ "${LEITZ_CUPS_DRIVER:-none}" != none ]] && ! lpstat -p LeitzIcon >/dev/null 2>&1; then
  lpadmin -p LeitzIcon -E \
    -v 'leitzusb:/dev/usb/lp0?sku=auto' \
    -P /usr/share/ppd/LeitzIcon.ppd \
    -o printer-is-shared=true
  echo 'INFO: Created LeitzIcon queue for integrated driver.'
fi

echo 'INFO: CUPS initialized. Admin: authenticated; LAN web interface: port 631.'
wait "$cups_pid"
