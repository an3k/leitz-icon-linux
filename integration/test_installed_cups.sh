#!/usr/bin/env bash
# Install the independent development driver into the Debian CUPS image
# built from the original, locally archived source on the cups branch.
set -euo pipefail
workspace=$(pwd -P)
shopt -s nullglob
versions=("$workspace"/frozen/sources/unpacked/cups-*)
(( ${#versions[@]} == 1 )) || {
  echo 'ERROR: Exactly one selected unpacked Debian CUPS version required' >&2
  exit 1
}
active=${versions[0]##*/}
active=${active#cups-}
grep -Fxq "Version: $active" "$workspace/frozen/sources/original/cups_$active.dsc"
# Exact image produced from those sources by a successful source-based build.
# A newer source revision must first publish its own verified CUPS image.
case "$active" in
  2.4.2-3+deb12u9)
    image='ghcr.io/an3k/leitz-icon-linux-cups@sha256:0d0e2469e307841b7fe2a5bdc9be4ffeb0b8ea11173696924a2dc9f75c21fb92'
    ;;
  *)
    echo "ERROR: No verified Debian CUPS integration image for $active" >&2
    exit 1
    ;;
esac
if [[ -n "${GH_TOKEN:-}" ]]; then
  printf '%s' "$GH_TOKEN" | docker login ghcr.io -u "${GITHUB_ACTOR:-github-actions}" --password-stdin >/dev/null
fi
docker pull "$image"
docker tag "$image" leitz-cups-frozen:verification

# Build the standalone development-driver package in a disposable Debian-CUPS
# container. The published base image remains unchanged.
docker run --rm --entrypoint /bin/bash \
  --mount "type=bind,source=$workspace/product,target=/work" \
  --workdir /work leitz-cups-frozen:verification -ec '
    apt-get update -qq
    apt-get install -y --no-install-recommends gcc make libcups2-dev python3
    bash install/package.sh
    test -s dist/leitz-icon-driver-linux-amd64.tar.gz
  '

# Repeat the exact install.sh distribution workflow in a separate,
# disposable container whose CUPS configuration starts untouched.
docker run --rm --entrypoint /bin/bash \
  --mount "type=bind,source=$workspace/product/dist/leitz-icon-driver-linux-amd64.tar.gz,target=/tmp/driver.tar.gz,readonly" \
  --env "CUPS_REQUIRED_VERSION=$active" leitz-cups-frozen:verification -ec '
    set -euo pipefail
    test "$(dpkg-query -W cups | cut -f2)" = "$CUPS_REQUIRED_VERSION"
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
    ppd_result=$(cupstestppd /usr/share/ppd/LeitzIcon.ppd 2>&1 || true)
    printf "%s\n" "$ppd_result"
    grep -Fq "/usr/share/ppd/LeitzIcon.ppd: PASS" <<< "$ppd_result"
    after=$(sha256sum /etc/cups/cupsd.conf)
    test "$original" = "${after%% *}"
    # The original CUPS archive remains immutable. In this disposable test
    # container only, add the local CUPS Unix socket so root can administer
    # its isolated queue without HTTP credentials or remote privileges.
    mkdir -p /run/cups
    printf "\\nListen /run/cups/cups.sock\\n" >> /etc/cups/cupsd.conf
    export CUPS_SERVER=/run/cups/cups.sock
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
    if ! lpadmin -h /run/cups/cups.sock -p LeitzIcon -E \
      -v "leitzusb:/dev/usb/lp0?sku=auto" \
      -P /usr/share/ppd/LeitzIcon.ppd \
      -o printer-error-policy=stop-printer; then
      cat /tmp/leitz-cupsd.log >&2
      test ! -f /var/log/cups/error_log || tail -n 40 /var/log/cups/error_log >&2
      exit 1
    fi
    lpstat -p LeitzIcon
    test "$(stat -c %a /usr/lib/cups/backend/leitzusb)" = 700
    echo "PASS: development driver registered with Debian CUPS $CUPS_REQUIRED_VERSION"
  '
