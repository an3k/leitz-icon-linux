#!/bin/bash
# Original macOS classic versus joined original-derived raster source.
# The final RGB page row is white, so the mode-1 encoder never sends
# its originally undefined final-row +4 byte tail.
set -uo pipefail
work="$1"; vendor="$2"; ppd="$3"
candidate="reconstructions/macos-5.1.465-classic/raster2printer/original_raster2printer_cups_candidate"
fail=0
for height in 33 80; do
 for cutter in ON OFF; do
  opts="Resolution=300dpi PrintQuality=Bayer8x8 OptimizationLevel=1 LabelCutter=$cutter PrintThroughPRN=OFF DebugMode=Release"
  PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$vendor" 703 tester RGB1 1 "$opts" "$work/rgb-tailblank-$height.ras" >"$work/rgbmode1-vendor.out" 2>"$work/rgbmode1-vendor.err"
  a=$?
  PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$candidate" 703 tester RGB1 1 "$opts" "$work/rgb-tailblank-$height.ras" >"$work/rgbmode1-joined.out" 2>"$work/rgbmode1-joined.err"
  b=$?
  if [[ "$a" == "$b" ]] && cmp -s "$work/rgbmode1-vendor.out" "$work/rgbmode1-joined.out" && cmp -s "$work/rgbmode1-vendor.err" "$work/rgbmode1-joined.err"; then
   echo "OEM_CONNECTED_MODE1_RGB_PARITY=YES ROWS=$height CUTTER=$cutter EXIT=$a BYTES=$(wc -c < "$work/rgbmode1-vendor.out" | tr -d ' ')"
  else
   echo "OEM_CONNECTED_MODE1_RGB_PARITY=NO ROWS=$height CUTTER=$cutter OEM_EXIT=$a C_EXIT=$b"
   cmp -l "$work/rgbmode1-vendor.out" "$work/rgbmode1-joined.out" | head -n 20 || true
   diff -u "$work/rgbmode1-vendor.err" "$work/rgbmode1-joined.err" | head -n 100 || true
   fail=1
  fi
 done
done
exit "$fail"
