#!/bin/bash
# Original and separately reconstructed joined macOS CUPS mode-1 jobs.
# For both pages the final row is entirely empty, so the original
# +4-byte raw copy on every SENT nonblank row stays within allocation.
# Unlike mode 0, mode 1 never sends the final blank row.
set -uo pipefail
work="$1"; vendor="$2"; ppd="$3"
joined="reconstructions/macos-5.1.465-classic/raster2printer/original_raster2printer_cups_candidate"
test -x "$vendor" && test -x "$joined" && test -f "$ppd" || exit 1
errors=0
for rows in 33 80; do
  for cutter in ON OFF; do
    options="Resolution=300dpi PrintQuality=Draft OptimizationLevel=1 LabelCutter=$cutter PrintThroughPRN=OFF DebugMode=Release"
    PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$vendor" \
      702 tester Mixed 1 "$options" "$work/tailblank-$rows.ras" \
      >"$work/mode1-mixed-vendor.out" 2>"$work/mode1-mixed-vendor.err"
    a=$?
    PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$joined" \
      702 tester Mixed 1 "$options" "$work/tailblank-$rows.ras" \
      >"$work/mode1-mixed-joined.out" 2>"$work/mode1-mixed-joined.err"
    b=$?
    if [[ "$a" == "$b" ]] &&
       cmp -s "$work/mode1-mixed-vendor.out" "$work/mode1-mixed-joined.out" &&
       cmp -s "$work/mode1-mixed-vendor.err" "$work/mode1-mixed-joined.err"; then
      echo "OEM_CONNECTED_MODE1_MIXED_PARITY=YES ROWS=$rows CUTTER=$cutter EXIT=$a BYTES=$(wc -c < "$work/mode1-mixed-vendor.out" | tr -d ' ')"
    else
      echo "OEM_CONNECTED_MODE1_MIXED_PARITY=NO ROWS=$rows CUTTER=$cutter OEM_EXIT=$a C_EXIT=$b"
      echo "First binary-byte differences:"
      cmp -l "$work/mode1-mixed-vendor.out" "$work/mode1-mixed-joined.out" | head -n 20 || true
      echo "First diagnostic differences:"
      diff -u "$work/mode1-mixed-vendor.err" "$work/mode1-mixed-joined.err" | head -n 130 || true
      errors=1
    fi
  done
done
exit "$errors"
