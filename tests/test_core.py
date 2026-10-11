#!/usr/bin/env python3
"""Self-contained offline reference and corruption checks; no printer required."""
from pathlib import Path
from subprocess import run
from tempfile import TemporaryDirectory

ROOT = Path(__file__).resolve().parent.parent
with TemporaryDirectory() as td:
    root = Path(td)
    pgm = root / "test.pgm"
    # Synthetic 960-pixel, 300x600 DPI raster; deliberately not the physical golden fixture.
    width, height = 960, 120
    with pgm.open("wb") as f:
        f.write(f"P5\n{width} {height}\n255\n".encode())
        for y in range(height):
            line = bytearray(b"\xff" * width)
            if y in (15, 119):
                line[10:30] = b"\x00" * 20
            f.write(line)
    dest = root / "test.bin"
    p = run([str(ROOT / "leitz-cli"), "pgm", str(pgm), str(dest)], capture_output=True)
    assert p.returncode == 0, p.stderr.decode()
    original = dest.read_bytes()
    assert run([str(ROOT / "leitz-cli"), "validate", str(dest)], capture_output=True).returncode == 0
    # Existing byte stream stays identical; only an explicitly requested
    # no-cut boundary can replace the intermediate page trailer.
    trailer = b"\x1bG\x1bE\x1b@\x1bZ"
    assert original.endswith(trailer)
    joined = original[:-len(trailer)] + b"\x1bG" + original
    combined = root / "two_pages_final_cut.bin"
    combined.write_bytes(joined)
    assert run([str(ROOT / "leitz-cli"), "validate", str(combined)],
               capture_output=True).returncode == 0
    combined.write_bytes(joined[:-len(trailer)] + b"\x1bG")
    assert run([str(ROOT / "leitz-cli"), "validate", str(combined)],
               capture_output=True).returncode != 0
    # USB must never receive a syntactically framed but blank or invalid
    # raster row: the spool validator is an independent safety boundary.
    prefix = original[:135]
    for body in (
        b"\x1bf\x00\x01",                       # Feed-only label
        b"\x1bD\x01\x1bB\x00\x16\x00", # Empty raw raster row
        b"\x1bD\x01\x1bB\x00\x17\x07", # All-white compressed row
        b"\x1bf\x00\x00\x1bD\x01\x1bB\x00\x16\x80", # Zero feed
    ):
        bad = root / "blank_or_invalid.bin"
        bad.write_bytes(prefix + body + trailer)
        assert run([str(ROOT / "leitz-cli"), "validate", str(bad)],
                   capture_output=True).returncode != 0
    for drop in (1, 9, 150):
        bad = root / "bad.bin"
        bad.write_bytes(original[:-drop])
        assert run([str(ROOT / "leitz-cli"), "validate", str(bad)], capture_output=True).returncode != 0
    print("PASS generated 300x600 synthetic sample and corruption rejection")
