#!/bin/bash
# No hardware access. Direct execution of SHA-256-verified manufacturer Mach-O
# and the provisional C reconstruction on the same CUPS raster input.
set -uo pipefail
work="$1"; original="$2"; ppd="$3"
native="sources/macos-5.1.465-classic/recovery/raster/classic_raster_cups_adapter"
all_ok=1
for mode in 0 1 2 3; do
  opts="Resolution=300dpi PrintQuality=Draft OptimizationLevel=$mode LabelCutter=ON PrintThroughPRN=OFF DebugMode=Release"
  PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$original" 101 user Test 1 "$opts" "$work/input.ras" >"$work/o-$mode.bin" 2>"$work/o-$mode.err"
  original_rc=$?
  PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$native" 101 user Test 1 "$opts" "$work/input.ras" >"$work/r-$mode.bin" 2>"$work/r-$mode.err"
  recovered_rc=$?
  original_length="$(wc -c < "$work/o-$mode.bin" | tr -d ' ')"
  recovered_length="$(wc -c < "$work/r-$mode.bin" | tr -d ' ')"
  echo "MODE=$mode ORIGINAL_EXIT=$original_rc RECON_EXIT=$recovered_rc ORIGINAL_BYTES=$original_length RECON_BYTES=$recovered_length"
  if [[ "$original_rc" == "$recovered_rc" ]] && cmp -s "$work/o-$mode.bin" "$work/r-$mode.bin"; then
    echo "MODE=$mode EXACT_OUTPUT=YES"
  else
    echo "MODE=$mode EXACT_OUTPUT=NO"
    if [[ "$mode" == 0 || "$mode" == 1 ]] && \
       [[ "$original_rc" == "$recovered_rc" ]] && \
       python3 sources/macos-5.1.465-classic/recovery/validation/compare_defined_bytes.py \
       "$work/o-$mode.bin" "$work/r-$mode.bin"; then
      echo "MODE=$mode DEFINED_OUTPUT=YES (OEM final-row data is undefined)"
      continue
    fi
    echo "OEM diagnostics:"
    tail -n 12 "$work/o-$mode.err"
    echo "Recovered diagnostics:"
    tail -n 12 "$work/r-$mode.err"
    echo "First 48 OEM output bytes:"
    od -An -tx1 -N48 "$work/o-$mode.bin"
    echo "First 48 reconstructed output bytes:"
    od -An -tx1 -N48 "$work/r-$mode.bin"
    echo "First differing bytes, if present:"
    cmp -l "$work/o-$mode.bin" "$work/r-$mode.bin" | head -n 12 || true
    if [[ "$mode" == 0 ]]; then
      python3 - "$work/o-0.bin" "$work/r-0.bin" <<'PY'
import pathlib,sys
a,b=(pathlib.Path(x).read_bytes() for x in sys.argv[1:3])
# 122-byte document start, 11-byte page start and 4-byte preamble.
for y in (0,1,2,3,4,7,79):
    old=137+92*y
    current=137+88*y
    print("RAW_ROW",y,"OEM88_EQUALS_RECON88",a[old:old+88]==b[current:current+88],
          "OEM_FOUR_TRAILING",a[old+88:old+92].hex(" "),
          "RECON_NEXT_FOUR",b[current+88:current+92].hex(" "))
PY
    fi
    all_ok=0
  fi
done
[[ "$all_ok" == 1 ]]
