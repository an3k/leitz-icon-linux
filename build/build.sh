#!/usr/bin/env bash
# Consume only the previously archived upstream source release.
# Never refresh CUPS or alter stored originals as part of a build.
set -euo pipefail
[[ -f source/source.lock ]] || {
  echo 'ERROR: No frozen CUPS source. Run upgrade before build.' >&2; exit 1;
}
source source/source.lock
[[ "$SOURCE_DIGEST" =~ ^sha256:[0-9a-f]{64}$ ]] || exit 1
[[ "$SOURCE_RELEASE" == "cups-source-${SOURCE_DIGEST#sha256:}" ]] || exit 1
[[ "$UPSTREAM_IMAGE" == 'docker.io/olbat/cupsd:latest' ]] || exit 1
workspace=$(mktemp -d)
trap 'rm -rf -- "$workspace"' EXIT
# Build uses the unpacked archive stored in this branch, not a moving tag
# and not a mutable GitHub Release attachment.
bash build/reconstruct.sh unpacked "$workspace/cups-unpacked.tar.gz"
docker load -i "$workspace/cups-unpacked.tar.gz"
docker tag "$UPSTREAM_IMAGE" leitz-cups-frozen:source

# 'release' is an independent checkout of the approved main branch.
[[ -f release/Makefile && -f release/cups/LeitzIcon.ppd ]] || {
  echo 'ERROR: Missing main release checkout' >&2; exit 1;
}
docker build --build-arg BASE_IMAGE=leitz-cups-frozen:source \
  -f build/Dockerfile -t leitz-cups-complete:build .
docker run --rm --entrypoint /bin/bash leitz-cups-complete:build -ec '
  test -x /usr/lib/cups/filter/leitz-rastertoicon
  test -x /usr/lib/cups/filter/leitz-cli
  test -x /usr/lib/cups/backend/leitzusb
  cupstestppd /usr/share/ppd/LeitzIcon.ppd
  test -x /usr/sbin/cupsd
'
mkdir -p dist
docker save leitz-cups-complete:build | gzip -1 > dist/leitz-icon-cups-unraid-image.tar.gz
printf '%s\n' "$SOURCE_DIGEST" > dist/cups-source-digest.txt
printf '%s\n' "$RELEASE_SHA" > dist/leitz-main-commit.txt
sha256sum dist/leitz-icon-cups-unraid-image.tar.gz > dist/SHA256SUMS
