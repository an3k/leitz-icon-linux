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
container_id="leitz-cups-verify-${BASHPID}"
trap 'docker rm -f "$container_id" >/dev/null 2>&1 || :; docker run --rm --entrypoint /bin/chmod --volume "$tmpdir:/verify" leitz-cups-frozen:verification -R a+rwX /verify >/dev/null 2>&1 || :; rm -rf -- "$tmpdir"' EXIT
dpkg-source -x "$original/cups_$active.dsc" "$tmpdir/cups-$active" >/dev/null
diff -qr "$tmpdir/cups-$active" "$unpacked/cups-$active" || {
  echo 'ERROR: Unpacked sources differ from verified Debian source package' >&2
  exit 1
}
echo "PASS: $archive_count archived originals with individual SHA256; exact extracted Debian CUPS $active"

# Build the actual driver-free stage, not the Dockerfile's default
# cups-with-driver final stage.
docker build --target cups-base --build-arg "CUPS_VERSION=$active" \
  -f "$frozen/build/Dockerfile" -t leitz-cups-frozen:verification "$frozen"

# Exercise the real CUPS entrypoint with persistent /etc/cups across a restart.
# This catches mismatched initialization marker names and accidental resets.
mkdir -p "$tmpdir/config" "$tmpdir/spool"
docker run -d --name "$container_id" \
  -e CUPS_ADMIN_USER=admin -e CUPS_ADMIN_PASSWORD=TemporaryTestPassword42 \
  --mount "type=bind,source=$tmpdir/config,target=/etc/cups" \
  --mount "type=bind,source=$tmpdir/spool,target=/var/spool/cups" \
  leitz-cups-frozen:verification >/dev/null
ready=0
for _ in $(seq 1 30); do
  if docker exec "$container_id" test -f /etc/cups/.cups-initialized &&
     docker exec "$container_id" lpstat -r 2>/dev/null | grep -qx 'scheduler is running'; then
    ready=1; break
  fi
  sleep 1
done
if (( !ready )); then docker logs "$container_id" >&2; exit 1; fi
! docker exec "$container_id" test -e /etc/cups/.leitz-initialized
! docker exec "$container_id" lpstat -p LeitzIcon >/dev/null 2>&1
docker exec "$container_id" /bin/bash -c "printf '\n# LEITZ_CUPS_CONFIG_PRESERVATION_SENTINEL\n' >> /etc/cups/cupsd.conf"
docker restart "$container_id" >/dev/null
ready=0
for _ in $(seq 1 30); do
  if docker exec "$container_id" lpstat -r 2>/dev/null | grep -qx 'scheduler is running'; then
    ready=1; break
  fi
  sleep 1
done
if (( !ready )); then docker logs "$container_id" >&2; exit 1; fi
docker exec "$container_id" grep -Fq '# LEITZ_CUPS_CONFIG_PRESERVATION_SENTINEL' /etc/cups/cupsd.conf
docker exec "$container_id" test -f /etc/cups/.cups-initialized
docker exec "$container_id" chmod -R ugo+rwX /etc/cups /var/spool/cups
docker rm -f "$container_id" >/dev/null
echo 'PASS: CUPS none start/restart preserves configuration, no preinstalled driver or queue'

# Build the standalone development-driver package in a disposable Debian-CUPS
# container. The published base image remains unchanged.
docker run --rm --entrypoint /bin/bash \
  --mount "type=bind,source=$workspace/product,target=/work" \
  --workdir /work leitz-cups-frozen:verification -ec '
    apt-get update -qq
    apt-get install -y --no-install-recommends gcc make libcups2-dev python3
    bash install/package.sh
    (cd dist && sha256sum leitz-icon-driver-linux-amd64.tar.gz > leitz-icon-driver-linux-amd64.tar.gz.sha256)
    test -s dist/leitz-icon-driver-linux-amd64.tar.gz.sha256
  '

# A driver-free image must also reinstall a mounted driver at each start,
# including when /etc/cups was persisted across a container recreation.
mkdir -p "$tmpdir/config-mounted" "$tmpdir/spool-mounted"
docker run -d --name "$container_id" \
  -e CUPS_ADMIN_USER=admin -e CUPS_ADMIN_PASSWORD=TemporaryTestPassword42 \
  --mount "type=bind,source=$tmpdir/config-mounted,target=/etc/cups" \
  --mount "type=bind,source=$tmpdir/spool-mounted,target=/var/spool/cups" \
  --mount "type=bind,source=$workspace/product/dist,target=/opt/leitz-driver,readonly" \
  leitz-cups-frozen:verification >/dev/null
for _ in $(seq 1 30); do
  if docker exec "$container_id" test -x /usr/lib/cups/filter/leitz-rastertoicon; then break; fi
  sleep 1
done
docker exec "$container_id" test -x /usr/lib/cups/filter/leitz-rastertoicon
docker exec "$container_id" test -x /usr/lib/cups/backend/leitzusb
docker exec "$container_id" test -f /usr/lib/leitz/protocols/leitz-native.so
docker restart "$container_id" >/dev/null
for _ in $(seq 1 30); do
  if docker exec "$container_id" test -x /usr/lib/cups/filter/leitz-rastertoicon; then break; fi
  sleep 1
done
docker exec "$container_id" test -x /usr/lib/cups/filter/leitz-rastertoicon
docker exec "$container_id" test -s /usr/share/ppd/LeitzIcon.ppd
docker exec "$container_id" chmod -R ugo+rwX /etc/cups /var/spool/cups
docker rm -f "$container_id" >/dev/null
echo 'PASS: mounted standalone driver automatically installed on container startup and restart'

# Repeat the exact install.sh distribution workflow in a separate,
# disposable container whose CUPS configuration starts untouched.
docker run --rm --entrypoint /bin/bash \
  --mount "type=bind,source=$workspace/product/dist/leitz-icon-driver-linux-amd64.tar.gz,target=/tmp/driver.tar.gz,readonly" \
  --env "CUPS_REQUIRED_VERSION=$active" leitz-cups-frozen:verification -ec '
    set -euo pipefail
    test "$(dpkg-query -W cups | cut -f2)" = "$CUPS_REQUIRED_VERSION"
    test "$LEITZ_CUPS_DRIVER" = none
    test ! -e /usr/lib/cups/filter/leitz-rastertoicon
    test ! -e /usr/lib/cups/backend/leitzusb
    test ! -e /usr/share/ppd/LeitzIcon.ppd
    test ! -e /usr/lib/leitz/protocols/leitz-current.so
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
    # Existing queue PPD must be refreshed by reinstalling the new driver.
    echo "# LEITZ_OBSOLETE_ACTIVE_PPD" >> /etc/cups/ppd/LeitzIcon.ppd
    grep -Fq LEITZ_OBSOLETE_ACTIVE_PPD /etc/cups/ppd/LeitzIcon.ppd
    /tmp/leitz-install/install.sh
    ! grep -Fq LEITZ_OBSOLETE_ACTIVE_PPD /etc/cups/ppd/LeitzIcon.ppd
    lpstat -p LeitzIcon
    test "$(stat -c %a /usr/lib/cups/backend/leitzusb)" = 700
    echo "PASS: development driver registered with Debian CUPS $CUPS_REQUIRED_VERSION"
  '
