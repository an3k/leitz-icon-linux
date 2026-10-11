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
verify_unpacked() {
  local version=$1 dir="$unpacked/cups-$1" actual
  [[ -f "$dir/debian/changelog" && -f "$dir/scheduler/main.c" ]] || return 1
  actual=$(dpkg-parsechangelog -l "$dir/debian/changelog" -S Version) || return 1
  [[ "$actual" == "$version" ]]
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
  if ! verify_unpacked "$version"; then
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
