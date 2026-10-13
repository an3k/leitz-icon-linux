#!/bin/bash
# New scenarios only: variable-length CUPS pages, no per-label cutting,
# original classic raster2printer vs recovered stdout raster adapter.
set -uo pipefail
work="$1"; original="$2"; ppd="$3"
native="reconstructions/macos-5.1.465-classic/raster2printer/classic_raster_cups_adapter"
failure=0
for height in 33 127; do
  for mode in 2 3; do
    options="Resolution=300dpi PrintQuality=Draft OptimizationLevel=$mode LabelCutter=OFF PrintThroughPRN=OFF DebugMode=Release"
    PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" \
      "$original" 33 tester Dynamic 1 "$options" "$work/mono-$height.ras" \
      > "$work/dynamic.o" 2> "$work/dynamic.o.err"
    original_rc=$?
    PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" \
      "$native" 33 tester Dynamic 1 "$options" "$work/mono-$height.ras" \
      > "$work/dynamic.r" 2> "$work/dynamic.r.err"
    recovered_rc=$?
    if [[ "$original_rc" == "$recovered_rc" ]] &&
       cmp -s "$work/dynamic.o" "$work/dynamic.r";then
      echo "DYNAMIC_PARITY=YES HEIGHT=$height MODE=$mode CUTTER=OFF"
    else
      echo "DYNAMIC_PARITY=NO HEIGHT=$height MODE=$mode CUTTER=OFF OEM_EXIT=$original_rc RECON_EXIT=$recovered_rc"
      echo "OEM_BYTES=$(wc -c < "$work/dynamic.o") RECON_BYTES=$(wc -c < "$work/dynamic.r")"
      cmp -l "$work/dynamic.o" "$work/dynamic.r" | head -n 12 || true
      tail -n 8 "$work/dynamic.o.err"
      tail -n 8 "$work/dynamic.r.err"
      failure=1
    fi
  done
done
exit "$failure"
