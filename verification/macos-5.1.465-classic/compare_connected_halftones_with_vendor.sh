#!/bin/bash
# Compare already reconstructed OEM RGB24 halftone tables when run through
# the *joined* original-derived macOS CUPS filter, not the older adapter.
# Only optimization modes 2/3: manufacturer 0/1 can read past final row.
# Covers all non-Draft OEM quality names other than Bayer8x8 (already checked).
set -uo pipefail
work="$1"; original="$2"; ppd="$3"
candidate="reconstructions/macos-5.1.465-classic/raster2printer/original_raster2printer_cups_candidate"
test -x "$original" && test -x "$candidate" && test -f "$ppd" || exit 1
bad=0
count=0
for quality in Bayer8x8SuperCell1 Bayer8x8SuperCell2 SuperCell1 SuperCell2 \
               SuperCell3 CUPSThresold64x64 SuperCell Bayer4x4 Bayer14x14 \
               Clustered8x8 Dispersed8x8 Nasik4x4 Dithering3x3; do
  for resolution in 300dpi 600dpi; do
    for mode in 2 3; do
      opts="Resolution=$resolution PrintQuality=$quality OptimizationLevel=$mode LabelCutter=ON PrintThroughPRN=OFF DebugMode=Release"
      PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$original" \
        704 tester Halftone 1 "$opts" "$work/rgb.ras" \
        >"$work/halftone-oem.bin" 2>"$work/halftone-oem.err"
      rc_original=$?
      PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$candidate" \
        704 tester Halftone 1 "$opts" "$work/rgb.ras" \
        >"$work/halftone-joined.bin" 2>"$work/halftone-joined.err"
      rc_joined=$?
      if [[ "$rc_original" == "$rc_joined" ]] &&
         cmp -s "$work/halftone-oem.bin" "$work/halftone-joined.bin" &&
         cmp -s "$work/halftone-oem.err" "$work/halftone-joined.err"; then
        echo "OEM_CONNECTED_HALFTONE_PARITY=YES QUALITY=$quality RESOLUTION=$resolution MODE=$mode BYTES=$(wc -c < "$work/halftone-oem.bin" | tr -d ' ')"
        count=$((count+1))
      else
        echo "OEM_CONNECTED_HALFTONE_PARITY=NO QUALITY=$quality RESOLUTION=$resolution MODE=$mode OEM_EXIT=$rc_original C_EXIT=$rc_joined"
        echo "First stdout discrepancies:"
        cmp -l "$work/halftone-oem.bin" "$work/halftone-joined.bin" | head -n 12 || true
        echo "First full stderr discrepancies:"
        diff -u "$work/halftone-oem.err" "$work/halftone-joined.err" | head -n 100 || true
        bad=1
      fi
    done
  done
done
echo "OEM_CONNECTED_HALFTONE_VERIFIED=$count OF=52"
exit "$bad"
