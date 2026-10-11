#!/usr/bin/env bash
# The only operation that may acquire a new upstream CUPS source snapshot.
# Captures both the unmodified Docker archive and its unpacked layer contents.
set -euo pipefail
: "${GITHUB_REPOSITORY:?}"
: "${GH_TOKEN:?}"
: "${GITHUB_SHA:?}"
source cups/upstream.env
[[ "$UPSTREAM_IMAGE" == docker.io/olbat/cupsd:latest ]] || {
  echo 'ERROR: Unexpected upstream image; inspect and approve source changes.' >&2; exit 1;
}
docker pull --platform linux/amd64 "$UPSTREAM_IMAGE"
ref=$(docker image inspect --format '{{index .RepoDigests 0}}' "$UPSTREAM_IMAGE")
[[ "$ref" =~ ^[^[:space:]]+@sha256:([0-9a-f]{64})$ ]] || {
  echo "ERROR: Cannot identify immutable upstream digest: $ref" >&2; exit 1;
}
digest="sha256:${BASH_REMATCH[1]}"
tag="cups-source-${BASH_REMATCH[1]}"
old=''
if [[ -f cups/source.lock ]]; then
  old=$(sed -n 's/^SOURCE_DIGEST=//p' cups/source.lock)
fi
if [[ "$old" == "$digest" ]]; then
  echo "Upstream source is unchanged: $digest"
  exit 0
fi
if ! gh release view "$tag" >/dev/null 2>&1; then
  workspace=$(mktemp -d)
  trap 'rm -rf -- "$workspace"' EXIT
  docker save -o "$workspace/cups-original.tar" "$UPSTREAM_IMAGE"
  mkdir -p "$workspace/unpacked"
  tar -xf "$workspace/cups-original.tar" -C "$workspace/unpacked"
  # Snapshot contents stay unchanged. Extraction is performed once here;
  # consumers may extract immutable archived copies in ephemeral workspaces.
  mkdir -p "$workspace/dist"
  gzip -1 -c "$workspace/cups-original.tar" > "$workspace/dist/cups-original.tar.gz"
  tar -czf "$workspace/dist/cups-unpacked.tar.gz" -C "$workspace/unpacked" .
  (cd "$workspace/dist" && sha256sum cups-original.tar.gz cups-unpacked.tar.gz > SHA256SUMS)
  gh release create "$tag" "$workspace/dist/cups-original.tar.gz" \
    "$workspace/dist/cups-unpacked.tar.gz" "$workspace/dist/SHA256SUMS" \
    --target "$GITHUB_SHA" --title "Frozen upstream CUPS source ${digest}" \
    --notes "Unmodified upstream Docker image: ${ref}. linux/amd64. This is a source snapshot, not a Leitz driver release."
fi
lock=$(printf 'UPSTREAM_IMAGE=%s\nSOURCE_REFERENCE=%s\nSOURCE_DIGEST=%s\nSOURCE_RELEASE=%s\n' \
  "$UPSTREAM_IMAGE" "$ref" "$digest" "$tag")
if [[ -f cups/source.lock ]] && cmp -s <(printf '%s' "$lock") cups/source.lock; then
  exit 0
fi
file_url="repos/$GITHUB_REPOSITORY/contents/cups/source.lock"
old_sha=$(gh api "$file_url" --jq .sha 2>/dev/null || :)
payload=$(printf '%s\n' "$lock" | base64 -w 0)
if [[ -n "$old_sha" ]]; then
  gh api --method PUT "$file_url" -f branch=cups -f sha="$old_sha" \
    -f message="chore(cups): pin immutable upstream source $digest" \
    -f content="$payload" >/dev/null
else
  gh api --method PUT "$file_url" -f branch=cups \
    -f message="chore(cups): pin immutable upstream source $digest" \
    -f content="$payload" >/dev/null
fi
echo "Pinned original CUPS release $tag; existing source archives were not changed."
