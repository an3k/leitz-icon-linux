#!/usr/bin/env bash
# Independent source/layout check and end-to-end CUPS integration.
# CUPS comes from selected Debian archives, not a hardcoded image digest.
set -euo pipefail
workspace=$(pwd -P)
frozen="$workspace/frozen"
original="$frozen/sources/original"
unpacked="$frozen/sources/unpacked"
shopt -s nullglob
versions=("$unpacked"/cups-*)
(( ${#versions[@]} == 1 )) && [[ -d "${versions[0]}" ]] || {
  echo 'ERROR: Exactly one unpacked Debian CUPS version is required' >&2
  exit 1
}
active=${versions[0]##*/}
active=${active#cups-}
[[ -f "$original/cups_$active.dsc" && ! -e "$frozen/source" ]] || {
  echo 'ERROR: Missing selected Debian .dsc or obsolete source/ directory present' >&2
  exit 1
}
[[ -f "$frozen/release/Makefile" && -f "$frozen/release/cups/LeitzIcon.ppd" ]] || {
  echo 'ERROR: Approved main checkout missing from frozen/release/' >&2
  exit 1
}
archives=("$original"/cups_*)
(( ${#archives[@]} >= 8 )) || { echo 'ERROR: Insufficient original source files' >&2; exit 1; }
archive_count=0
hash_count=0
for file in "${archives[@]}"; do
  [[ -f "$file" ]] || { echo 'ERROR: Non-file in original archive directory' >&2; exit 1; }
  if [[ "$file" == *.sha256 ]]; then
    (( hash_count+=1 ))
    [[ -s "${file%.sha256}" ]] || { echo "ERROR: Orphan checksum: $file" >&2; exit 1; }
  else
    (( archive_count+=1 ))
    [[ -s "$file.sha256" ]] || { echo "ERROR: Missing checksum: $file" >&2; exit 1; }
    (cd "$original" && sha256sum --status -c "${file##*/}.sha256") || {
      echo "ERROR: Invalid checksum: $file" >&2
      exit 1
    }
  fi
done
(( archive_count == hash_count && archive_count >= 4 )) || {
  echo 'ERROR: Incorrect archived-source/checksum pairing' >&2; exit 1
}
upstream=${active%%-*}
for name in "cups_$upstream.orig.tar.gz" "cups_$upstream.orig.tar.gz.asc" \
            "cups_$active.debian.tar.xz" "cups_$active.dsc"; do
  [[ -s "$original/$name" && -s "$original/$name.sha256" ]] || {
    echo "ERROR: Incomplete selected source: $name" >&2; exit 1
  }
done
grep -Fxq "Version: $active" "$original/cups_$active.dsc"
[[ "$(dpkg-parsechangelog -l "$unpacked/cups-$active/debian/changelog" -S Version)" == "$active" ]]
tmpdir=$(mktemp -d)
trap 'rm -rf -- "$tmpdir"' EXIT
dpkg-source -x "$original/cups_$active.dsc" "$tmpdir/cups-$active" >/dev/null
diff -qr "$tmpdir/cups-$active" "$unpacked/cups-$active" || {
  echo 'ERROR: Unpacked sources differ from verified Debian source package' >&2
  exit 1
}
echo "PASS: $archive_count archived originals with individual SHA256; exact extracted Debian CUPS $active"

# Build this source revision with the approved main driver as a temporary
# base. Newer restore/upgrade versions need no manual digest edits in tests.
docker build --build-arg "CUPS_VERSION=$active" \
  -f "$frozen/build/Dockerfile" -t leitz-cups-frozen:verification "$frozen"

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
