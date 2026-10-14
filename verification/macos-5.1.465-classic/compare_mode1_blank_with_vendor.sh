#!/bin/bash
# Authentic macOS OEM and joined CUPS reconstruction: original mode 1,
# all-blank two-page raster. No raw row is transmitted, so the original
# extra four-byte read after a nonblank final row is never reached.
# All three complete process outputs are compared, not just raster bytes.
set -uo pipefail
work="$1"; original="$2"; ppd="$3"
candidate="reconstructions/macos-5.1.465-classic/raster2printer/original_raster2printer_cups_candidate"
test -x "$original" && test -x "$candidate" && test -f "$ppd" || exit 1
fail=0
for height in 1 33 127; do
  for cutter in ON OFF; do
    opts="Resolution=300dpi PrintQuality=Draft OptimizationLevel=1 LabelCutter=$cutter PrintThroughPRN=OFF DebugMode=Release"
    PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" \
      "$original" 701 tester Blank 1 "$opts" "$work/blank-$height.ras" \
      >"$work/blank-oem.out" 2>"$work/blank-oem.err"
    vendor_rc=$?
    PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" \
      "$candidate" 701 tester Blank 1 "$opts" "$work/blank-$height.ras" \
      >"$work/blank-connected.out" 2>"$work/blank-connected.err"
    connected_rc=$?
    if [[ "$vendor_rc" == "$connected_rc" ]] &&
       cmp -s "$work/blank-oem.out" "$work/blank-connected.out" &&
       cmp -s "$work/blank-oem.err" "$work/blank-connected.err"; then
      echo "OEM_CONNECTED_MODE1_BLANK_PARITY=YES ROWS=$height CUTTER=$cutter EXIT=$vendor_rc BYTES=$(wc -c < "$work/blank-oem.out" | tr -d ' ')"
    else
      echo "OEM_CONNECTED_MODE1_BLANK_PARITY=NO ROWS=$height CUTTER=$cutter OEM_EXIT=$vendor_rc C_EXIT=$connected_rc"
      echo "Vendor stdout size:";wc -c < "$work/blank-oem.out"
      echo "Candidate stdout size:";wc -c < "$work/blank-connected.out"
      cmp -l "$work/blank-oem.out" "$work/blank-connected.out" | head -n 18 || true
      diff -u "$work/blank-oem.err" "$work/blank-connected.err" | head -n 95 || true
      fail=1
    fi
  done
done
exit "$fail"
