#!/bin/bash
# Original mode 0 reads four bytes past each final packed raster row.
# This is manufacturer undefined behavior. Compare only independently
# defined bytes, exact stderr and exit status; never invent the tail.
set -uo pipefail
work="$1"; oem="$2"; ppd="$3"
joined="reconstructions/macos-5.1.465-classic/raster2printer/original_raster2printer_cups_candidate"
test -x "$oem" && test -x "$joined" && test -f "$ppd" || exit 1
failed=0
matched=0
for format in mono rgb; do
  for height in 33 80; do
    input="$work/mode0-$format-$height.ras"
    for cutter in ON OFF; do
      quality=Draft
      if [[ "$format" == "rgb" ]]; then quality=Bayer8x8; fi
      options="Resolution=300dpi PrintQuality=$quality OptimizationLevel=0 LabelCutter=$cutter PrintThroughPRN=OFF DebugMode=Release"
      PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$oem" \
        711 tester Mode0 1 "$options" "$input" >"$work/mode0-oem.out" 2>"$work/mode0-oem.err"
      rc_oem=$?
      PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$joined" \
        711 tester Mode0 1 "$options" "$input" >"$work/mode0-joined.out" 2>"$work/mode0-joined.err"
      rc_joined=$?
      if [[ "$rc_oem" == "$rc_joined" ]] &&
         cmp -s "$work/mode0-oem.err" "$work/mode0-joined.err" &&
         python3 verification/macos-5.1.465-classic/compare_defined_bytes.py \
           "$work/mode0-oem.out" "$work/mode0-joined.out"; then
        echo "OEM_CONNECTED_MODE0_DEFINED_PARITY=YES FORMAT=$format ROWS=$height CUTTER=$cutter EXIT=$rc_oem"
        matched=$((matched+1))
      else
        echo "OEM_CONNECTED_MODE0_DEFINED_PARITY=NO FORMAT=$format ROWS=$height CUTTER=$cutter OEM_EXIT=$rc_oem C_EXIT=$rc_joined"
        diff -u "$work/mode0-oem.err" "$work/mode0-joined.err" | head -n 75 || true
        failed=1
      fi
    done
  done
done
echo "OEM_CONNECTED_MODE0_DEFINED_VERIFIED=$matched OF=8"
exit "$failed"
