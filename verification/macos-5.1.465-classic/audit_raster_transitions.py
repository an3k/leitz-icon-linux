#!/usr/bin/env python3
"""Bounded OEM disassembly for still-unreconstructed page/loop transitions.

Run only on the hash-verified original raster2printer Mach-O using the
macOS GitHub runner. No patching, source generation or binary alteration.
"""
import re
import subprocess
import sys

if len(sys.argv) != 2:
    raise SystemExit("usage: audit_raster_transitions.py <verified-oem-raster2printer>")

mach_o = sys.argv[1]
result = subprocess.run(["otool", "-tvV", mach_o], check=True,
                        text=True, capture_output=True).stdout
sections = (
    ("AFTER_START_DOCUMENT_TO_PAGE_READ", 0x100005470, 0x1000056d0),
    ("AFTER_READ_TO_PAGE_OUTPUT", 0x100005ea0, 0x100005f90),
    ("AFTER_PAGE_OUTPUT_TO_EPILOGUE", 0x100006120, 0x100006360),
)
asm = []
for line in result.splitlines():
    match = re.match(r"^\s*(?:0x)?([0-9a-f]{10,16})\s+(.+)$", line)
    if match:
        asm.append((int(match.group(1), 16), line))
for name, begin, end in sections:
    lines = [line for addr, line in asm if begin <= addr < end]
    print("ORIGINAL_RASTER_TRANSITION_BEGIN", name,
          hex(begin), hex(end), "instructions", len(lines))
    # Output bounded to protect Actions logs; no extrapolation from truncation.
    for line in lines[:550]:
        print(line)
    if len(lines) > 550:
        print("ORIGINAL_RASTER_TRANSITION_TRUNCATED", name, len(lines) - 550)
    print("ORIGINAL_RASTER_TRANSITION_END", name)

# Original compiler locals governing loop behavior. Inventory *every*
# direct reference in the native _raster2printer body, not just a few
# selected blocks; do not assume a variable stays constant from a
# source-code reading of one branch.
print("ORIGINAL_RASTER_LOOP_SLOTS_BEGIN")
slots = ("-0x7c05(%rbp)", "-0x7c06(%rbp)", "-0x7c19(%rbp)",
         "-0x7bf0(%rbp)", "-0x7bf4(%rbp)")
for address, line in asm:
    if 0x1000045a0 <= address < 0x1000062eb and any(
        s in line for s in slots
    ):
        print(line)
print("ORIGINAL_RASTER_LOOP_SLOTS_END")

# All direct calls within the active native function. Use this to
# distinguish already reconstructed print helpers from still-unlinked
# status, URI, and raw output helpers.
print("ORIGINAL_RASTER_ACTIVE_CALLS_BEGIN")
for address, line in asm:
    if 0x1000045a0 <= address < 0x10000685a and (
        "\\tcallq\\t" in line or "\\tcall\\t" in line
    ):
        print(line)
print("ORIGINAL_RASTER_ACTIVE_CALLS_END")
