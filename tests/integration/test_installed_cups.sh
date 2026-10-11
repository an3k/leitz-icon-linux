#!/usr/bin/env bash
# This independent suite installs a driver-only package into a temporary
# container created from the original frozen Community Applications image.
# All original source archives and the driver repository are read-only inputs.
set -euo pipefail
workspace=$(pwd -P)
source "$workspace/frozen/cups/source.lock"
[[ "$SOURCE_DIGEST" =~ ^sha256:[a-f0-9]{64}$ ]] || exit 1
mkdir -p "$workspace/frozen/download"
gh release download "$SOURCE_RELEASE" -R "$GITHUB_REPOSITORY" --dir "$workspace/frozen/download" \
  -p cups-unpacked.tar.gz -p SHA256SUMS
(cd "$workspace/frozen/download" && sha256sum --ignore-missing -c SHA256SUMS)
docker load -i "$workspace/frozen/download/cups-unpacked.tar.gz"
docker tag "$UPSTREAM_IMAGE" leitz-cups-frozen:verification

# Build a distributable archive in a disposable container based on the
# frozen upstream image. Do not install build dependencies in the original.
docker run --rm --entrypoint /bin/bash \
  --mount "type=bind,source=$workspace/product,target=/work" \
  --workdir /work leitz-cups-frozen:verification -ec '
    apt-get update -qq
    apt-get install -y --no-install-recommends gcc make libcups2-dev python3
    bash driver/install/package.sh
    test -s dist/leitz-icon-driver-linux-amd64.tar.gz
  '

# Repeat the exact install.sh distribution workflow in a separate,
# disposable container whose CUPS configuration starts untouched.
docker run --rm --entrypoint /bin/bash \
  --mount "type=bind,source=$workspace/product/dist/leitz-icon-driver-linux-amd64.tar.gz,target=/tmp/driver.tar.gz,readonly" \
  leitz-cups-frozen:verification -ec '
    set -euo pipefail
    original=$(sha256sum /etc/cups/cupsd.conf)
    original=${original%% *}
    mkdir -p /tmp/leitz-install
    tar -xzf /tmp/driver.tar.gz -C /tmp/leitz-install
    /tmp/leitz-install/install.sh
    test -x /usr/lib/cups/filter/leitz-rastertoicon
    test -x /usr/lib/cups/filter/leitz-cli
    test -x /usr/lib/cups/filter/leitz-command
    test -x /usr/lib/cups/backend/leitzusb
    test -f /usr/lib/leitz/protocols/leitz-current.so
    test -f /usr/lib/leitz/protocols/leitz-native.so
    cupstestppd /usr/share/ppd/LeitzIcon.ppd
    after=$(sha256sum /etc/cups/cupsd.conf)
    test "$original" = "${after%% *}"
    # The stock upstream image listens on TCP port 631. CUPS command-line
    # defaults may instead try a missing local domain socket.
    export CUPS_SERVER=127.0.0.1:631
    /usr/sbin/cupsd -f > /tmp/leitz-cupsd.log 2>&1 &
    cupsd_pid=$!
    trap "kill -TERM $cupsd_pid 2>/dev/null || true" EXIT
    ready=0
    for n in $(seq 1 20); do
      if lpstat -r 2>/dev/null | grep -qx "scheduler is running"; then ready=1; break; fi
      if ! kill -0 "$cupsd_pid" 2>/dev/null; then break; fi
      sleep 1
    done
    if test "$ready" != 1; then
      cat /tmp/leitz-cupsd.log >&2
      test ! -f /var/log/cups/error_log || tail -n 40 /var/log/cups/error_log >&2
      exit 1
    fi
    if ! lpadmin -h 127.0.0.1:631 -p LeitzIcon -E \
      -v "leitzusb:/dev/usb/lp0?sku=auto" \
      -P /usr/share/ppd/LeitzIcon.ppd \
      -o printer-error-policy=stop-printer; then
      cat /tmp/leitz-cupsd.log >&2
      test ! -f /var/log/cups/error_log || tail -n 40 /var/log/cups/error_log >&2
      exit 1
    fi
    lpstat -p LeitzIcon
    test "$(stat -c %a /usr/lib/cups/backend/leitzusb)" = 700
    echo "PASS: production driver installed and registered in unchanged CUPS source"
  '
