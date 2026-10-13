#!/bin/bash
# Side-by-side original classic PRN transfer, in an ephemeral macOS CI runner.
# The script never opens a network or hardware device.
set -uo pipefail
work="$1"
original="$2"
ppd="$3"
native="sources/macos-5.1.465-classic/recovery/raster/classic_raster_cups_adapter"
prn="/Library/Caches/DEBUG/dbg.prn"
opts="Resolution=300dpi PrintQuality=Draft OptimizationLevel=2 LabelCutter=ON PrintThroughPRN=ON DebugMode=Release"

# The OEM really uses this absolute path. Run on a disposable macOS runner
# only; never prepare this location on the user's workstation or server.
sudo mkdir -p /Library/Caches/DEBUG || exit 1
sudo chown "$(id -un)" /Library/Caches/DEBUG || exit 1
sudo chmod 0755 /Library/Caches/DEBUG || exit 1

PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$original" 301 tester PRN 1 "$opts" "$work/input.ras" > "$work/oem-prn.out" 2> "$work/oem-prn.err"
a=$?
if [[ -f "$prn" ]]; then
  cp "$prn" "$work/oem-prn.dat"
else
  : > "$work/oem-prn.dat"
fi

PPD="$ppd" DEVICE_URI="usb://Leitz-Icon" "$native" 301 tester PRN 1 "$opts" "$work/input.ras" > "$work/recovered-prn.out" 2> "$work/recovered-prn.err"
b=$?
if [[ -f "$prn" ]]; then
  cp "$prn" "$work/recovered-prn.dat"
else
  : > "$work/recovered-prn.dat"
fi

echo "PRN OEM_EXIT=$a RECON_EXIT=$b OEM_STDOUT_BYTES=$(wc -c < "$work/oem-prn.out") RECON_STDOUT_BYTES=$(wc -c < "$work/recovered-prn.out")"
echo "PRN OEM_FILE_BYTES=$(wc -c < "$work/oem-prn.dat") RECON_FILE_BYTES=$(wc -c < "$work/recovered-prn.dat")"
if [[ "$a" == "$b" ]] &&
   [[ -s "$work/oem-prn.out" ]] &&
   [[ -s "$work/oem-prn.dat" ]] &&
   cmp -s "$work/oem-prn.out" "$work/recovered-prn.out" &&
   cmp -s "$work/oem-prn.dat" "$work/recovered-prn.dat"; then
  echo "PRN ORIGINAL_PARITY=YES"
  exit 0
fi
echo "PRN ORIGINAL_PARITY=NO"
echo "Original stderr:";tail -n 15 "$work/oem-prn.err"
echo "Reconstructed stderr:";tail -n 15 "$work/recovered-prn.err"
echo "First differing stdout bytes:"
cmp -l "$work/oem-prn.out" "$work/recovered-prn.out" | head -n 12 || true
exit 1
