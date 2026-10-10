#!/usr/bin/env python3
"""Offline source-protocol isolation test: no printer, no raster input needed."""
from pathlib import Path
import subprocess
import tempfile

CLI = Path(__file__).resolve().parents[1] / "leitz-cli"
ESC = b"\x1b"
native = (
    ESC + b"@" + ESC + b"i"                # manufacturer Linux 600 dpi
    + ESC * 120
    + ESC + b"L" + b"\x00\x03"
    + ESC + b"w" + b"\x00\x10"          # 16 pixels
    + ESC + b"B\x00"
    + ESC + b"f\x00\x01"               # one blank line before data
    + ESC + b"D\x02\x16\x80\x00"      # packed raw row
    + ESC + b"G" + ESC + b"E"
)

def check(mode, path, should_pass):
    result = subprocess.run([str(CLI), mode, str(path)],
                            capture_output=True, text=True, timeout=5)
    assert (result.returncode == 0) == should_pass, (
        mode, result.returncode, result.stderr
    )
    return result.stderr

with tempfile.TemporaryDirectory() as tmp:
    path = Path(tmp) / "native.bin"
    path.write_bytes(native)
    message = check("validate-native", path, True)
    assert "pages=1; width=16" in message, message
    check("validate", path, False)  # No silent interchange of wire protocols.

    # Real manufacturer document behavior: only the first page resets the
    # printer, and LabelCutter=OFF skips ESC E between pages, not at job end.
    second_page = native[4:]  # Same resolution, page header, width and dots.
    no_intermediate_cut = native[:-2] + second_page
    path.write_bytes(no_intermediate_cut)
    assert "pages=2; width=16" in check("validate-native", path, True)
    check("validate", path, False)
    cut_each = native + second_page
    path.write_bytes(cut_each)
    assert "pages=2; width=16" in check("validate-native", path, True)

    path.write_bytes(no_intermediate_cut[:-2])
    check("validate-native", path, False)  # Never omit final ESC E.
    path.write_bytes(no_intermediate_cut + b"\x00")
    check("validate-native", path, False)

    path.write_bytes(native[:-1])  # Missing final ESC E byte.
    check("validate-native", path, False)
    path.write_bytes(native + b"\x00")  # Extra data after native footer.
    check("validate-native", path, False)

    path.write_bytes(b"")
    check("validate-native", path, False)

print("PASS original Linux native wire stream accepted only by explicit offline validator")
