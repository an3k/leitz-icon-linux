#!/usr/bin/env bash
set -euo pipefail
if (( $# != 1 )) || [[ ! "$1" =~ ^[0-9][0-9A-Za-z.+~:-]*$ ]]; then
  echo 'Usage: bash build/restore.sh <Debian-package-version>' >&2; exit 2
fi
source "$(dirname "$0")/source-common.sh"
cd "$repo_root"
[[ "$(git branch --show-current)" == cups ]] || { echo 'ERROR: cups branch required.' >&2; exit 1; }
version=$1
verify_originals "$version" || { echo "ERROR: No archived originals for $version" >&2; exit 1; }
extract_version "$version"
activate_version "$version"
commit_sources "source(cups): restore Debian CUPS $version"
echo "Selected archived Debian CUPS $version"
