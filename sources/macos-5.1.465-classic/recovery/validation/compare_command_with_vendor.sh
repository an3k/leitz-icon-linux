#!/bin/bash
# Real OEM command2leitz vs recovered entry, synthetic replies only.
# Do not use attached printers. All output is captured to temporary files.
set -uo pipefail
work="$1"
original="$2"
ppd="$3"
module="$work/mock_backchannel.dylib"
new_exec="sources/macos-5.1.465-classic/recovery/command2leitz_recovered_cups"

clang -std=c11 -Wall -Wextra -Werror -dynamiclib \
  -o "$module" \
  sources/macos-5.1.465-classic/recovery/validation/mock_backchannel.c -lcups || exit 1

make -C sources/macos-5.1.465-classic/recovery \
  command2leitz_recovered_cups CC=clang \
  CFLAGS="-O2 -std=c11 -Wall -Wextra -Werror -Wno-deprecated-declarations" || exit 1

errors=0
for scenario in "ReportStatus READY" "ReportStatus NONE" \
                "ReportStatus MEDIA_EMPTY" "ReportStatus MEDIA_JAM" \
                "ReportStatus GENERAL_ERROR" "ReportStatus UNKNOWN_STATUS" \
                "ReportStatus READ_ERROR" "GetUnitedStatus U_42" \
                "GetFirmwareVersion V_42" "GetUnitedStatus NONE" \
                "ReportState NONE"; do
  command="$(echo "$scenario" | cut -d' ' -f1)"
  reply="$(echo "$scenario" | cut -d' ' -f2)"
  printf '%s YES\n' "$command" > "$work/control.conf"
  PPD="$ppd" LEITZ_MOCK_REPLY="$reply" \
     DYLD_INSERT_LIBRARIES="$module" \
     "$original" 201 testuser Classic 1 "" "$work/control.conf" \
     > "$work/command.original" 2> "$work/command.original.err"
  original_status=$?
  PPD="$ppd" LEITZ_MOCK_REPLY="$reply" \
     DYLD_INSERT_LIBRARIES="$module" \
     "$new_exec" 201 testuser Classic 1 "" "$work/control.conf" \
     > "$work/command.recovered" 2> "$work/command.recovered.err"
  recovered_status=$?
  echo "COMMAND=$command REPLY=$reply OEM_EXIT=$original_status RECOVERED_EXIT=$recovered_status"
  if [[ "$original_status" == "$recovered_status" ]] &&
     cmp -s "$work/command.original" "$work/command.recovered"; then
    echo "COMMAND=$command REPLY=$reply STDOUT_PARITY=YES"
  else
    echo "COMMAND=$command REPLY=$reply STDOUT_PARITY=NO"
    echo "OEM bytes:";od -An -tx1 "$work/command.original"
    echo "Recovered bytes:";od -An -tx1 "$work/command.recovered"
    errors=1
  fi
  grep -E '^((STATE:)|(ATTR:))' "$work/command.original.err" \
    > "$work/o.events" || true
  grep -E '^((STATE:)|(ATTR:))' "$work/command.recovered.err" \
    > "$work/r.events" || true
  if cmp -s "$work/o.events" "$work/r.events"; then
    echo "COMMAND=$command REPLY=$reply CUPS_EVENTS_PARITY=YES"
  else
    echo "COMMAND=$command REPLY=$reply CUPS_EVENTS_PARITY=NO"
    echo "OEM events:";cat "$work/o.events"
    echo "Recovered events:";cat "$work/r.events"
    errors=1
  fi
  if [[ "$reply" != "NONE" && "$command" != "ReportState" ]]; then
    if grep -q '^MOCK_BACKCHANNEL:' "$work/command.original.err"; then
      echo "COMMAND=$command MOCK_INTERPOSITION=CONFIRMED"
    else
      echo "COMMAND=$command MOCK_INTERPOSITION=NOT_CONFIRMED"
      errors=1
    fi
  fi
done
exit "$errors"
