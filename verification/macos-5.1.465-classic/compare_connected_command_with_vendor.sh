#!/bin/bash
# Genuine 5.1.465 OEM command2leitz vs independently linked reconstructed
# _main -> _GetStatus path, with a local CUPS backchannel interposer.
# No printer or network device is contacted. All three streams are compared.
set -uo pipefail
work="$1"; original="$2"; ppd="$3"
candidate="reconstructions/macos-5.1.465-classic/command2leitz/command2leitz_original_main_candidate"
module="$work/mock_backchannel.dylib"
test -x "$original" && test -x "$candidate" && test -f "$module" || exit 1
errors=0
for case in "ReportStatus READY" "ReportStatus NONE" "ReportStatus MEDIA_JAM" \
            "GetUnitedStatus NONE" "GetFirmwareVersion NONE" \
            "GetUnitedStatus U_42" \
            "ReportStatus READ_ERROR" "ReportStatus MEDIA_EMPTY" \
            "ReportStatus GENERAL_ERROR" "ReportStatus UNKNOWN_STATUS" \
            "GetUnitedStatus READ_ERROR" "GetFirmwareVersion READ_ERROR" \
            "GetFirmwareVersion V_42"; do
  read -r command answer <<< "$case"
  printf '%s YES\n' "$command" > "$work/command-original-main.conf"
  PPD="$ppd" LEITZ_MOCK_REPLY="$answer" DYLD_INSERT_LIBRARIES="$module" \
    "$original" 201 testuser Classic 1 "" "$work/command-original-main.conf" \
    > "$work/command-main-vendor.out" 2> "$work/command-main-vendor.err"
  a=$?
  PPD="$ppd" LEITZ_MOCK_REPLY="$answer" DYLD_INSERT_LIBRARIES="$module" \
    "$candidate" 201 testuser Classic 1 "" "$work/command-original-main.conf" \
    > "$work/command-main-joined.out" 2> "$work/command-main-joined.err"
  b=$?
  if [[ "$a" == "$b" ]] &&
     cmp -s "$work/command-main-vendor.out" "$work/command-main-joined.out" &&
     cmp -s "$work/command-main-vendor.err" "$work/command-main-joined.err"; then
    echo "OEM_COMMAND_CONNECTED_PARITY=YES COMMAND=$command RESPONSE=$answer EXIT=$a"
  else
    echo "OEM_COMMAND_CONNECTED_PARITY=NO COMMAND=$command RESPONSE=$answer OEM_EXIT=$a CONNECTED_EXIT=$b"
    echo "stdout original:";od -An -tx1 "$work/command-main-vendor.out"
    echo "stdout joined:";od -An -tx1 "$work/command-main-joined.out"
    echo "stderr diff:"
    diff -u "$work/command-main-vendor.err" "$work/command-main-joined.err" || true
    errors=1
  fi
  if ! grep -q '^MOCK_BACKCHANNEL:' "$work/command-main-vendor.err" ||
     ! grep -q '^MOCK_BACKCHANNEL:' "$work/command-main-joined.err"; then
    echo "OEM_COMMAND_CONNECTED_INTERPOSER=NOT_CONFIRMED COMMAND=$command"
    errors=1
  fi
done
exit "$errors"
