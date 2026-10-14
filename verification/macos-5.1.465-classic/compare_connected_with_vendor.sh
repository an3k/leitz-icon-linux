#!/bin/bash
# Execute BOTH binaries offline against the same CUPS raster reference input.
# Uses only modes 2/3: the original's modes 0/1 read undefined last-row tail.
# A mismatch is evidence of remaining reconstruction differences, not a
# reason to change the established OEM model without further disassembly.
set -uo pipefail
work="$1"; oem="$2"; ppd="$3"
candidate="reconstructions/macos-5.1.465-classic/raster2printer/original_raster2printer_cups_candidate"
failed=0
for input in input.ras rgb.ras; do
  for mode in 2 3; do
    if [[ "$input" == "rgb.ras" ]]; then
      quality=Bayer8x8
    else
      quality=Draft
    fi
    opts="Resolution=300dpi PrintQuality=$quality OptimizationLevel=$mode LabelCutter=ON PrintThroughPRN=OFF DebugMode=Release"
    PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$oem" 101 tester Compare 1 "$opts" \
      "$work/$input" >"$work/oem-native.bin" 2>"$work/oem-native.err"
    original_rc=$?
    PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$candidate" 101 tester Compare 1 "$opts" \
      "$work/$input" >"$work/candidate-native.bin" 2>"$work/candidate-native.err"
    candidate_rc=$?
    if [[ "$original_rc" == "$candidate_rc" ]] &&
       cmp -s "$work/oem-native.bin" "$work/candidate-native.bin" &&
       cmp -s "$work/oem-native.err" "$work/candidate-native.err"; then
      echo "OEM_CONNECTED_C_PARITY=YES RASTER=$input MODE=$mode EXIT=$original_rc STDOUT_AND_STDERR=IDENTICAL BYTES=$(wc -c < "$work/oem-native.bin" | tr -d ' ')"
    else
      echo "OEM_CONNECTED_C_PARITY=NO RASTER=$input MODE=$mode OEM_EXIT=$original_rc C_EXIT=$candidate_rc"
      echo "OEM_BYTES=$(wc -c < "$work/oem-native.bin") C_BYTES=$(wc -c < "$work/candidate-native.bin")"
      cmp -l "$work/oem-native.bin" "$work/candidate-native.bin" | head -n 15 || true
      echo "Complete original-vs-connected stderr first 100 differing lines:"
      diff -u "$work/oem-native.err" "$work/candidate-native.err" | head -n 120 || true
      echo "Original stderr:";tail -n 10 "$work/oem-native.err"
      echo "Connected C stderr:";tail -n 10 "$work/candidate-native.err"
      failed=1
    fi
  done
done
exit "$failed"
