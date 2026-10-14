#!/bin/bash
# Original 5.1.465 Mach-O vs native, source-derived CUPS raster executable.
# Compare start-up/error paths, including complete stdout, stderr and exit.
# No network, printer, or physical cutter is involved.
set -uo pipefail
work="$1"; original="$2"; ppd="$3"
candidate="reconstructions/macos-5.1.465-classic/raster2printer/original_raster2printer_cups_candidate"
test -x "$original" && test -x "$candidate" && test -f "$ppd" || exit 1
touch "$work/empty-startup.ras"
errors=0
for scenario in argc_five argc_eight missing_input missing_ppd empty_input stdin_eof; do
  effective_ppd="$ppd"
  case "$scenario" in
    argc_five) set -- 601 tester Error 1 ;;
    argc_eight) set -- 601 tester Error 1 "OptimizationLevel=2" "$work/input.ras" unexpected ;;
    missing_input) set -- 601 tester Error 1 "OptimizationLevel=2" "$work/missing-startup.ras" ;;
    missing_ppd)
      effective_ppd="$work/missing-startup.ppd"
      set -- 601 tester Error 1 "OptimizationLevel=2" "$work/input.ras" ;;
    empty_input) set -- 601 tester Error 1 "OptimizationLevel=2" "$work/empty-startup.ras" ;;
    stdin_eof) set -- 601 tester Error 1 "OptimizationLevel=2" ;;
  esac
  PPD="$effective_ppd" DEVICE_URI="usb://Leitz-Icon" \
    "$original" "$@" </dev/null \
    >"$work/startup-vendor.out" 2>"$work/startup-vendor.err"
  original_status=$?
  PPD="$effective_ppd" DEVICE_URI="usb://Leitz-Icon" \
    "$candidate" "$@" </dev/null \
    >"$work/startup-reconstruction.out" 2>"$work/startup-reconstruction.err"
  reconstructed_status=$?
  if [[ "$original_status" == "$reconstructed_status" ]] &&
     cmp -s "$work/startup-vendor.out" "$work/startup-reconstruction.out" &&
     cmp -s "$work/startup-vendor.err" "$work/startup-reconstruction.err"; then
    echo "OEM_CONNECTED_STARTUP_PARITY=YES SCENARIO=$scenario EXIT=$original_status STDOUT_BYTES=$(wc -c < "$work/startup-vendor.out" | tr -d ' ')"
  else
    echo "OEM_CONNECTED_STARTUP_PARITY=NO SCENARIO=$scenario OEM_EXIT=$original_status C_EXIT=$reconstructed_status"
    echo "Vendor stdout:";od -An -tx1 "$work/startup-vendor.out" | head -n 15 || true
    echo "Reconstruction stdout:";od -An -tx1 "$work/startup-reconstruction.out" | head -n 15 || true
    echo "Complete stderr diff:"
    diff -u "$work/startup-vendor.err" "$work/startup-reconstruction.err" | head -n 120 || true
    errors=1
  fi
done
exit "$errors"
