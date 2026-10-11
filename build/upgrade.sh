#!/usr/bin/env bash
# Sole writer for frozen source snapshots; never edits an existing snapshot.
# Old original and unpacked archives remain available by digest in Git.
set -euo pipefail
: "${GH_TOKEN:?}"
: "${GITHUB_REPOSITORY:?}"
[[ "$(git branch --show-current)" == cups ]] || {
  echo 'ERROR: Upgrade requires a cups branch checkout.' >&2; exit 1
}
source source/upstream.env
[[ "$UPSTREAM_IMAGE" == docker.io/olbat/cupsd:latest ]] || exit 1
current=''
if [[ -f source/source.lock ]]; then
  current=$(sed -n 's/^SOURCE_DIGEST=//p' source/source.lock)
fi
workspace=$(mktemp -d)
trap 'rm -rf -- "$workspace"' EXIT

# The first migration imports the previously preserved source snapshot;
# there is no need to download the original upstream image a second time.
if [[ "$current" =~ ^sha256:[0-9a-f]{64}$ ]] &&
   [[ ! -f "source/${current#sha256:}/SHA256SUMS" ]]; then
  digest="$current"
  source source/source.lock
  ref="$SOURCE_REFERENCE"
  tag="$SOURCE_RELEASE"
  echo "Importing existing immutable source $tag into cups branch."
else
  docker pull --platform linux/amd64 "$UPSTREAM_IMAGE"
  ref=$(docker image inspect --format '{{index .RepoDigests 0}}' "$UPSTREAM_IMAGE")
  [[ "$ref" =~ ^[^[:space:]]+@sha256:([a-f0-9]{64})$ ]] || exit 1
  digest="sha256:${BASH_REMATCH[1]}"
  tag="cups-source-${BASH_REMATCH[1]}"
  if [[ "$digest" == "$current" ]] &&
     [[ -f "source/${digest#sha256:}/SHA256SUMS" ]]; then
    echo "CUPS frozen source is already current: $digest"
    exit 0
  fi
  if ! gh release view "$tag" -R "$GITHUB_REPOSITORY" >/dev/null 2>&1; then
    mkdir -p "$workspace/unpacked"
    docker save -o "$workspace/cups-original.tar" "$UPSTREAM_IMAGE"
    tar -xf "$workspace/cups-original.tar" -C "$workspace/unpacked"
    mkdir -p "$workspace/distribution"
    gzip -1 -c "$workspace/cups-original.tar" > "$workspace/distribution/cups-original.tar.gz"
    tar -czf "$workspace/distribution/cups-unpacked.tar.gz" -C "$workspace/unpacked" .
    (cd "$workspace/distribution" &&
      sha256sum cups-original.tar.gz cups-unpacked.tar.gz > SHA256SUMS)
    gh release create "$tag" "$workspace/distribution/cups-original.tar.gz" \
      "$workspace/distribution/cups-unpacked.tar.gz" "$workspace/distribution/SHA256SUMS" \
      -R "$GITHUB_REPOSITORY" \
      --target "$(git rev-parse HEAD)" \
      --title "Frozen upstream CUPS source $digest" \
      --notes "Unmodified linux/amd64 Docker source $ref. Retained for upstream provenance."
  fi
fi

snapshot="source/${digest#sha256:}"
if [[ -e "$snapshot" ]]; then
  echo 'ERROR: Existing snapshot cannot be changed or overwritten' >&2
  exit 1
fi
mkdir -p "$workspace/download"
gh release download "$tag" -R "$GITHUB_REPOSITORY" --dir "$workspace/download" \
  -p cups-original.tar.gz -p cups-unpacked.tar.gz -p SHA256SUMS
(cd "$workspace/download" && sha256sum -c SHA256SUMS)
mkdir -p "$snapshot/original" "$snapshot/unpacked"
cp "$workspace/download/SHA256SUMS" "$snapshot/SHA256SUMS"
for kind in original unpacked; do
  split -b 32m -d -a 3 "$workspace/download/cups-$kind.tar.gz" \
    "$snapshot/$kind/part-"
done

# GitHub rejects blobs over 100 MiB. Each visible, immutable part is 32 MiB.
# Incremental commits prevent a single oversized HTTP pack upload.
git config user.name 'github-actions[bot]'
git config user.email '41898282+github-actions[bot]@users.noreply.github.com'
while IFS= read -r part; do
  git add -- "$part"
  git commit -m "archive(cups): preserve $digest ${part#"$snapshot/"}"
  git push origin HEAD:refs/heads/cups
done < <(find "$snapshot" -type f | LC_ALL=C sort)

printf 'UPSTREAM_IMAGE=%s\nSOURCE_REFERENCE=%s\nSOURCE_DIGEST=%s\nSOURCE_RELEASE=%s\n' \
  "$UPSTREAM_IMAGE" "$ref" "$digest" "$tag" > source/source.lock
git add -- source/source.lock
git commit -m "chore(cups): select fully archived immutable source $digest"
git push origin HEAD:refs/heads/cups
echo "Frozen source and original archive are now visible under $snapshot."
