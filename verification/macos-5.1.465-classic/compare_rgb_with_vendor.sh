#!/bin/bash
# Original macOS classic raster vs reconstructed CUPS entry for RGB24 pages.
# No printer is addressed. A failure is an observed parity discrepancy.
set -uo pipefail
work="$1"
original="$2"
ppd="$3"
native="reconstructions/macos-5.1.465-classic/raster2printer/classic_raster_cups_adapter"
bad=0
# Fourth argument allows running only previously unverified PPD choices.
quality_set="Bayer8x8 Bayer4x4 SuperCell2 Dithering3x3"
if [[ $# -ge 4 ]];then quality_set="$4";fi
for quality in $quality_set; do
  for resolution in 300dpi 600dpi; do
    for mode in 0 1 2 3; do
      opts="Resolution=$resolution PrintQuality=$quality OptimizationLevel=$mode LabelCutter=ON PrintThroughPRN=OFF DebugMode=Release"
      PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$original" 201 tester RGB 1 "$opts" "$work/rgb.ras" > "$work/rgb-original.bin" 2> "$work/rgb-original.err"
      a=$?
      PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$native" 201 tester RGB 1 "$opts" "$work/rgb.ras" > "$work/rgb-recovered.bin" 2> "$work/rgb-recovered.err"
      b=$?
      original_size="$(wc -c < "$work/rgb-original.bin" | tr -d ' ')"
      recovered_size="$(wc -c < "$work/rgb-recovered.bin" | tr -d ' ')"
      case_name="quality=$quality resolution=$resolution mode=$mode"
      if [[ "$a" == "$b" ]] && cmp -s "$work/rgb-original.bin" "$work/rgb-recovered.bin";then
        echo "RGB24_PARITY=YES $case_name bytes=$original_size"
      elif [[ "$mode" == 0 || "$mode" == 1 ]] &&
           [[ "$a" == "$b" ]] &&
           python3 verification/macos-5.1.465-classic/compare_defined_bytes.py \
             "$work/rgb-original.bin" "$work/rgb-recovered.bin"; then
        echo "RGB24_DEFINED_PARITY=YES $case_name bytes=$original_size"
      else
        echo "RGB24_PARITY=NO $case_name oem_exit=$a recon_exit=$b oem_bytes=$original_size recon_bytes=$recovered_size"
        echo "First unequal offsets:"
        cmp -l "$work/rgb-original.bin" "$work/rgb-recovered.bin" | head -n 8 || true
        echo "OEM stderr tail:"
        tail -n 7 "$work/rgb-original.err"
        echo "Recovered stderr tail:"
        tail -n 7 "$work/rgb-recovered.err"
        bad=1
      fi
    done
  done
done
exit "$bad"
