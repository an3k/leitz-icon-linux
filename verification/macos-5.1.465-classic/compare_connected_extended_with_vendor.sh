#!/bin/bash
# Compare the authentic 5.1.465 manufacturer raster executable against the
# distinct joined OEM-source CUPS candidate for variable length and PRN mode.
# Runs only on the ephemeral macOS reference CI runner, without real hardware.
# The workflow's existing PRN test has prepared the original /Library/Caches
# directory; do not create or touch that path on a user's workstation.
set -uo pipefail
work="$1"; oem="$2"; ppd="$3"
candidate="reconstructions/macos-5.1.465-classic/raster2printer/original_raster2printer_cups_candidate"
prn="/Library/Caches/DEBUG/dbg.prn"
test -x "$oem" && test -x "$candidate" && test -f "$ppd" || exit 1
fail=0

for height in 33 127; do
  for mode in 2 3; do
    options="Resolution=300dpi PrintQuality=Draft OptimizationLevel=$mode LabelCutter=OFF PrintThroughPRN=OFF DebugMode=Release"
    PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" \
      "$oem" 301 tester Dynamic 1 "$options" "$work/mono-$height.ras" \
      >"$work/connected-variable-oem.bin" 2>"$work/connected-variable-oem.err"
    original_exit=$?
    PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" \
      "$candidate" 301 tester Dynamic 1 "$options" "$work/mono-$height.ras" \
      >"$work/connected-variable-reconstructed.bin" 2>"$work/connected-variable-reconstructed.err"
    connected_exit=$?
    if [[ "$original_exit" == "$connected_exit" ]] &&
       cmp -s "$work/connected-variable-oem.bin" "$work/connected-variable-reconstructed.bin"; then
      echo "OEM_CONNECTED_VARIABLE_PARITY=YES HEIGHT=$height MODE=$mode CUTTER=OFF BYTES=$(wc -c < "$work/connected-variable-oem.bin" | tr -d ' ')"
    else
      echo "OEM_CONNECTED_VARIABLE_PARITY=NO HEIGHT=$height MODE=$mode ORIGINAL_EXIT=$original_exit C_EXIT=$connected_exit"
      echo "OEM_BYTES=$(wc -c < "$work/connected-variable-oem.bin") CONNECTED_BYTES=$(wc -c < "$work/connected-variable-reconstructed.bin")"
      cmp -l "$work/connected-variable-oem.bin" "$work/connected-variable-reconstructed.bin" | head -n 16 || true
      tail -n 12 "$work/connected-variable-oem.err"
      tail -n 12 "$work/connected-variable-reconstructed.err"
      fail=1
    fi
  done
done

test -d "/Library/Caches/DEBUG" && test -w "/Library/Caches/DEBUG" || {
  echo "OEM_CONNECTED_PRN_PARITY=UNTESTED original cache is unavailable"
  exit 1
}
options="Resolution=300dpi PrintQuality=Draft OptimizationLevel=2 LabelCutter=ON PrintThroughPRN=ON DebugMode=Release"
PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" \
  "$oem" 301 tester PRN 1 "$options" "$work/input.ras" \
  >"$work/connected-prn-oem.out" 2>"$work/connected-prn-oem.err"
original_exit=$?
test -f "$prn" && cp "$prn" "$work/connected-prn-oem.dat" || exit 1
PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" \
  "$candidate" 301 tester PRN 1 "$options" "$work/input.ras" \
  >"$work/connected-prn-reconstructed.out" 2>"$work/connected-prn-reconstructed.err"
connected_exit=$?
test -f "$prn" && cp "$prn" "$work/connected-prn-reconstructed.dat" || exit 1
if [[ "$original_exit" == "$connected_exit" ]] &&
   [[ -s "$work/connected-prn-oem.dat" ]] &&
   cmp -s "$work/connected-prn-oem.out" "$work/connected-prn-reconstructed.out" &&
   cmp -s "$work/connected-prn-oem.dat" "$work/connected-prn-reconstructed.dat"; then
  echo "OEM_CONNECTED_PRN_PARITY=YES EXIT=$original_exit STDOUT_BYTES=$(wc -c < "$work/connected-prn-oem.out" | tr -d ' ') PRN_BYTES=$(wc -c < "$work/connected-prn-oem.dat" | tr -d ' ')"
else
  echo "OEM_CONNECTED_PRN_PARITY=NO OEM_EXIT=$original_exit C_EXIT=$connected_exit"
  echo "OEM_STDOUT_BYTES=$(wc -c < "$work/connected-prn-oem.out") CONNECTED_STDOUT_BYTES=$(wc -c < "$work/connected-prn-reconstructed.out")"
  echo "OEM_PRN_BYTES=$(wc -c < "$work/connected-prn-oem.dat") CONNECTED_PRN_BYTES=$(wc -c < "$work/connected-prn-reconstructed.dat")"
  cmp -l "$work/connected-prn-oem.out" "$work/connected-prn-reconstructed.out" | head -n 16 || true
  cmp -l "$work/connected-prn-oem.dat" "$work/connected-prn-reconstructed.dat" | head -n 16 || true
  tail -n 15 "$work/connected-prn-oem.err"
  tail -n 15 "$work/connected-prn-reconstructed.err"
  fail=1
fi
exit "$fail"
