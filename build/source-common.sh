#!/usr/bin/env bash
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)
original="$repo_root/sources/original"
unpacked="$repo_root/sources/unpacked"
mkdir -p "$original" "$unpacked"

source_filenames() {
  local version=$1 upstream
  upstream=${version%%-*}
  printf '%s\n' "cups_$upstream.orig.tar.gz" "cups_$upstream.orig.tar.gz.asc" \
    "cups_$version.debian.tar.xz" "cups_$version.dsc"
}
verify_originals() {
  local version=$1 name
  while IFS= read -r name; do
    [[ -s "$original/$name" && -s "$original/$name.sha256" ]] || return 1
    (cd "$original" && sha256sum --status -c "$name.sha256") || return 1
  done < <(source_filenames "$version")
  grep -Fxq "Version: $version" "$original/cups_$version.dsc"
}
# The OpenPrinting detached source signature is independently checked
# against the upstream signing key shipped by Debian in its patched source.
# Debian APT authenticates the source package metadata on download.
verify_upstream_signature() {
  local version=$1 source_dir=$2 archive_dir=$3 upstream keydir
  upstream=${version%%-*}
  [[ -s "$source_dir/debian/upstream/signing-key.asc" &&
     -s "$archive_dir/cups_$upstream.orig.tar.gz.asc" &&
     -s "$archive_dir/cups_$upstream.orig.tar.gz" ]] || return 1
  command -v gpg >/dev/null && command -v gpgv >/dev/null || {
    echo 'ERROR: GPG and GPGV are needed for CUPS source authentication' >&2
    return 1
  }
  keydir=$(mktemp -d) || return 1
  if ! gpg --batch --dearmor < "$source_dir/debian/upstream/signing-key.asc" > "$keydir/upstream.gpg" ||
     ! gpgv --keyring "$keydir/upstream.gpg" \
       "$archive_dir/cups_$upstream.orig.tar.gz.asc" \
       "$archive_dir/cups_$upstream.orig.tar.gz" >/dev/null 2>&1; then
    echo "ERROR: Original upstream CUPS signature invalid for $version" >&2
    rm -rf -- "$keydir"
    return 1
  fi
  rm -rf -- "$keydir"
}
verify_unpacked() {
  local version=$1 dir="$unpacked/cups-$1" actual
  [[ -f "$dir/debian/changelog" && -f "$dir/scheduler/main.c" ]] || return 1
  actual=$(dpkg-parsechangelog -l "$dir/debian/changelog" -S Version) || return 1
  [[ "$actual" == "$version" ]]
}
# Full byte-for-byte verification against the already authenticated Debian
# source package; do not build a tree whose contents differ from its archive.
verify_source_tree() {
  local version=$1 target="$unpacked/cups-$1" scratch
  verify_originals "$version" && verify_unpacked "$version" || return 1
  scratch=$(mktemp -d) || return 1
  if ! dpkg-source -x "$original/cups_$version.dsc" "$scratch/source" >/dev/null 2>&1 ||
     ! diff -qr -- "$scratch/source" "$target" >/dev/null; then
    rm -rf -- "$scratch"
    echo "ERROR: Unpacked Debian CUPS sources differ from archived originals ($version)" >&2
    return 1
  fi
  if ! verify_upstream_signature "$version" "$scratch/source" "$original"; then
    rm -rf -- "$scratch"
    return 1
  fi
  rm -rf -- "$scratch"
}
active_version() {
  local dirs=("$unpacked"/cups-*)
  (( ${#dirs[@]} == 1 )) || return 1
  [[ -d "${dirs[0]}" ]] || return 1
  local version
  version=$(basename "${dirs[0]}")
  version=${version#cups-}
  verify_unpacked "$version" || return 1
  printf '%s\n' "$version"
}
latest_local_version() {
  local file candidate best=''
  for file in "$original"/cups_*.dsc; do
    [[ -f "$file" ]] || continue
    candidate=$(basename "$file")
    candidate=${candidate#cups_}
    candidate=${candidate%.dsc}
    verify_originals "$candidate" || continue
    if [[ -z "$best" ]] || dpkg --compare-versions "$candidate" gt "$best"; then
      best=$candidate
    fi
  done
  [[ -n "$best" ]] || return 1
  printf '%s\n' "$best"
}
extract_version() {
  local version=$1 dir="$unpacked/cups-$1"
  verify_originals "$version" || { echo "ERROR: Invalid originals: $version" >&2; return 1; }
  if verify_unpacked "$version"; then return 0; fi
  rm -rf -- "$dir"
  if ! dpkg-source -x "$original/cups_$version.dsc" "$dir"; then
    rm -rf -- "$dir"; return 1
  fi
  if ! verify_unpacked "$version" ||
     ! verify_upstream_signature "$version" "$dir" "$original"; then
    rm -rf -- "$dir"; return 1
  fi
  # dpkg-source can create an upstream-archive sidecar beside its output directory.
  local upstream_version
  upstream_version=${version%%-*}
  rm -f -- "$unpacked/cups_$upstream_version.orig.tar.gz"
}
activate_version() {
  local version=$1 directory
  verify_unpacked "$version" || return 1
  for directory in "$unpacked"/*; do
    [[ -e "$directory" ]] || continue
    [[ "$directory" == "$unpacked/cups-$version" ]] || rm -rf -- "$directory"
  done
}
commit_sources() {
  local message=$1
  git config user.name 'github-actions[bot]'
  git config user.email '41898282+github-actions[bot]@users.noreply.github.com'
  git add -A -- sources/ requests/
  if ! git diff --cached --quiet; then
    git commit -m "$message"
    git push origin HEAD:refs/heads/cups
  fi
}
