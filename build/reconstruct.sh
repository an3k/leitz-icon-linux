#!/usr/bin/env bash
# Reconstruct exactly one stored, immutable CUPS source archive.
# Usage: reconstruct.sh original|unpacked OUTPUT_FILE
set -euo pipefail
if (($# != 2)); then
  echo 'Usage: reconstruct.sh original|unpacked output-file' >&2; exit 2
fi
kind=$1
[[ "$kind" == original || "$kind" == unpacked ]] || exit 2
root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)
source "$root/source/source.lock"
[[ "$SOURCE_DIGEST" =~ ^sha256:[a-f0-9]{64}$ ]] || exit 1
snapshot="$root/source/${SOURCE_DIGEST#sha256:}"
[[ -f "$snapshot/SHA256SUMS" ]] || {
  echo 'ERROR: Frozen CUPS archive is not stored in this branch.' >&2; exit 1
}
shopt -s nullglob
parts=("$snapshot/$kind"/part-[0-9][0-9][0-9])
(("${#parts[@]}" > 0)) || { echo 'ERROR: Source chunks missing' >&2; exit 1; }
for ((i=0;i<${#parts[@]};i++)); do
  printf -v number '%03d' "$i"
  [[ "${parts[i]}" == "$snapshot/$kind/part-$number" ]] || {
    echo 'ERROR: Source chunk sequence is incomplete' >&2; exit 1
  }
done
filename="cups-$kind.tar.gz"
expected=$(awk -v name="$filename" '$2==name {print $1}' "$snapshot/SHA256SUMS")
[[ "$expected" =~ ^[a-f0-9]{64}$ ]] || {
  echo 'ERROR: No checksum for frozen CUPS source' >&2; exit 1
}
cat -- "${parts[@]}" > "$2"
actual=$(sha256sum "$2")
if [[ "${actual%% *}" != "$expected" ]]; then
  echo 'ERROR: Immutable source checksum mismatch' >&2; exit 1
fi
echo "Validated frozen $kind CUPS source (${#parts[@]} chunks)."
