#!/usr/bin/env python3
"""Offline test for the existing leitz-cli status-snapshot command (never USB)."""
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CLI = ROOT / "leitz-cli"

def run(mode, data=None, expected_success=True, source=None):
    if source is None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "reply.bin"
            path.write_bytes(data)
            return run(mode, expected_success=expected_success, source=path)
    result = subprocess.run(
        [str(CLI), "decode-status", mode, str(source)],
        capture_output=True, text=True, check=False, timeout=5,
    )
    assert (result.returncode == 0) == expected_success, (
        mode, result.returncode, result.stdout, result.stderr
    )
    return result.stdout

out = run("lines", bytes([0, 0, 1, 44]))
assert "PRINTED_LINES_CANDIDATE=300" in out
assert "COMPLETION_CONFIRMED=NO" in out
out = run("lines", bytes([0]))
assert "PRINTED_LINES_CANDIDATE=0" in out
out = run("legacy", bytes([0x03]))
assert "STATUS_BYTE=0x03" in out and "READY_LEGACY" in out
out = run("v3", bytes([0xC0]))
assert "AMBIGUOUS=YES" in out and "FAULT_OR_BUSY" in out
out = run("version", b"v2.1\x00")
assert "VERSION_TEXT=v2.1" in out
run("lines", bytes([0, 0, 0, 0, 0]), expected_success=False)
run("version", b"v2\x00junk", expected_success=False)
run("lines", b"", expected_success=False)
run("version", b"A" * 1025, expected_success=False)
run("legacy", expected_success=False, source=Path("/dev/null"))
run("unsupported", b"A", expected_success=False)
print("PASS offline CLI response-snapshot decoding and unsafe input rejection")
