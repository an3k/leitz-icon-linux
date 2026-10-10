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
    assert "width=16; transmitted_rows=2" in message, message
    check("validate", path, False)  # No silent interchange of wire protocols.

    path.write_bytes(native[:-1])  # Missing final ESC E byte.
    check("validate-native", path, False)
    path.write_bytes(native + b"\x00")  # Extra data after native footer.
    check("validate-native", path, False)

    path.write_bytes(b"")
    check("validate-native", path, False)

print("PASS original Linux native wire stream accepted only by explicit offline validator")
