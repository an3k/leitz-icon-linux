#!/usr/bin/env bash
# Select latest Debian 12 CUPS source, keeping all valid original archives.
set -euo pipefail
source "$(dirname "$0")/source-common.sh"
cd "$repo_root"
[[ "$(git branch --show-current)" == cups ]] || { echo 'ERROR: cups branch required.' >&2; exit 1; }

# If the unpacked source is absent, restore a locally archived version first.
if ! active=$(active_version); then
  if local_version=$(latest_local_version); then
    extract_version "$local_version"
    activate_version "$local_version"
    commit_sources "source(cups): recover archived Debian CUPS $local_version"
    exit 0
  fi
  active=''
fi

retry_file=requests/.upgrade-failures
mkdir -p requests
now=$(date +%s)
cutoff=$((now - 43200))
if [[ -f "$retry_file" ]]; then
  awk -v cutoff="$cutoff" '$1 ~ /^[0-9]+$/ && $1 >= cutoff {print $1}' "$retry_file" > "$retry_file.tmp"
  mv "$retry_file.tmp" "$retry_file"
fi
failures=0
if [[ -f "$retry_file" ]]; then failures=$(wc -l < "$retry_file"); fi
if (( failures >= 3 )); then
  echo 'ERROR: Three failed download/verification attempts within 12 hours.' >&2
  exit 1
fi
record_failure() {
  date +%s >> "$retry_file"
  commit_sources 'chore(cups): record failed Debian CUPS download attempt'
}
workspace=$(mktemp -d)
trap 'rm -rf -- "$workspace"' EXIT
chmod 0777 "$workspace"

download_latest() {
  rm -rf -- "$workspace"/*
  docker run --rm --volume "$workspace:/work" --workdir /work debian:bookworm-slim \
    /bin/bash -euo pipefail -c '
      cat > /etc/apt/sources.list.d/cups-src.list <<EOF
deb-src http://deb.debian.org/debian bookworm main
deb-src http://deb.debian.org/debian bookworm-updates main
deb-src http://deb.debian.org/debian-security bookworm-security main
EOF
      apt-get update -qq
      latest=""
      while IFS= read -r candidate; do
        if [[ -z "$latest" ]] || dpkg --compare-versions "$candidate" gt "$latest"; then
          latest=$candidate
        fi
      done < <(apt-cache showsrc cups | sed -n "s/^Version: //p")
      [[ -n "$latest" ]]
      printf "%s\n" "$latest" > /work/.version
      apt-get source --download-only "cups=$latest"
    ' >&2
  local version name
  version=$(cat "$workspace/.version")
  while IFS= read -r name; do
    [[ -s "$workspace/$name" ]] || return 1
  done < <(source_filenames "$version")
  dpkg-source -x "$workspace/cups_$version.dsc" "$workspace/verified" >&2
  [[ "$(dpkg-parsechangelog -l "$workspace/verified/debian/changelog" -S Version)" == "$version" ]]
}
save_originals() {
  local version=$1 name
  while IFS= read -r name; do
    # Retain valid archived originals; replace corrupted downloads, not valid differing bytes.
    if [[ -e "$original/$name" && -f "$original/$name.sha256" ]] &&
       ! (cd "$original" && sha256sum --status -c "$name.sha256"); then
      rm -f -- "$original/$name" "$original/$name.sha256"
    fi
    if [[ -e "$original/$name" ]]; then
      if ! cmp -s "$workspace/$name" "$original/$name"; then
        echo "ERROR: Valid original file collision: $name" >&2
        return 1
      fi
    else
      cp -- "$workspace/$name" "$original/$name"
    fi
    if [[ ! -f "$original/$name.sha256" ]]; then
      (cd "$original" && sha256sum "$name" > "$name.sha256")
    fi
  done < <(source_filenames "$version")
  verify_originals "$version"
}

while (( failures < 3 )); do
  if ! download_latest; then
    echo 'ERROR: Debian source download or checksum validation failed.' >&2
    record_failure
    failures=$((failures + 1))
    continue
  fi
  version=$(cat "$workspace/.version")
  if [[ -n "$active" ]] && ! dpkg --compare-versions "$version" gt "$active"; then
    rm -f -- "$retry_file"
    commit_sources "chore(cups): source version checked ($active)"
    echo "Current Debian CUPS version already selected: $active"
    exit 0
  fi
  if ! save_originals "$version" || ! extract_version "$version"; then
    rm -rf -- "$unpacked/cups-$version"
    echo 'ERROR: Failed to verify or extract latest source.' >&2
    record_failure
    failures=$((failures + 1))
    continue
  fi
  activate_version "$version"
  rm -f -- "$retry_file"
  commit_sources "source(cups): activate Debian CUPS $version"
  echo "Activated Debian CUPS $version; preserved older original archives."
  exit 0
done
echo 'ERROR: Three failed source download/verification attempts in 12 hours.' >&2
exit 1
