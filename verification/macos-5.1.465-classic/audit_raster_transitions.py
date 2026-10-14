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

# The inlined active page reader is separate from exported _ReadPage.
# Inspect its actual quotient/remainder operators before propagating
# any correction: neighboring source code is NOT evidence of parity.
print("ORIGINAL_ACTIVE_PAGE_STRIDE_DIVISIONS_BEGIN")
for index,(addr,line) in enumerate(asm):
    if 0x1000056c0 <= addr < 0x100005ed0 and (
        "divl" in line or "idivl" in line
    ):
        print("OEM_INLINE_STRIDE_DIVISION",hex(addr))
        for _,neighbor in asm[max(0,index-10):min(len(asm),index+11)]:
            print(neighbor)
print("ORIGINAL_ACTIVE_PAGE_STRIDE_DIVISIONS_END")

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
direct_calls = [
    line for address, line in asm
    if 0x1000045a0 <= address < 0x10000685a and "callq" in line
]
if not direct_calls:
    raise RuntimeError("No OEM call instructions matched; audit is invalid")
for line in direct_calls:
    print(line)
print("ORIGINAL_RASTER_ACTIVE_CALLS_END")

# Manufacturer entry/exit functions are distinct from the job loop.
# Recover complete defined _EndDocument before implementing a native
# wrapper; the existing byte-only boundary C cannot model close/flush.
print("ORIGINAL_RASTER_DOCUMENT_BOUNDARY_BEGIN")
for name,begin,end in (
    ("START_DOCUMENT",0x100006890,0x100006dd0),
    ("END_DOCUMENT",0x100008ac0,0x100008cf0),
):
    subset=[line for address,line in asm if begin<=address<end]
    if not subset:
        raise RuntimeError("No disassembly for "+name)
    print("OEM_DOCUMENT_FUNCTION",name,"instructions",len(subset))
    for line in subset:
        print(line)
print("ORIGINAL_RASTER_DOCUMENT_BOUNDARY_END")

# Two previously byte-only original OEM page lifecycle functions.
# Include all native instructions, no truncation, after SHA256-verification.
print("ORIGINAL_RASTER_PAGE_BOUNDARIES_BEGIN")
for name, begin, end in (
    ("START_PAGE", 0x100007380, 0x100007560),
    ("END_PAGE", 0x1000089b0, 0x100008ac0),
):
    instructions = [line for address, line in asm if begin <= address < end]
    if not instructions:
        raise RuntimeError("Original page boundary missing: " + name)
    print("ORIGINAL_RASTER_PAGE_FUNCTION", name, "instructions", len(instructions))
    for line in instructions:
        print(line)
print("ORIGINAL_RASTER_PAGE_BOUNDARIES_END")

# Complete OEM _ProcessPage setup, row-boundary, return/termination regions.
# Capture every native instruction in the selected regions; other row
# encoding branches are already represented in the modes_01/modes_23 files.
print("ORIGINAL_PROCESS_PAGE_CONTROL_BEGIN")
for name, begin, end in (
    ("ENTRY_OPTIONS_AND_SETUP", 0x100007560, 0x1000078eb),
    ("OUTPUT_AND_PAGE_EXIT", 0x100008850, 0x1000089b0),
):
    region = [line for address, line in asm if begin <= address < end]
    if not region:
        raise RuntimeError("No original _ProcessPage instructions for "+name)
    print("OEM_PROCESS_CONTROL", name, "instructions", len(region))
    for line in region:
        print(line)
print("ORIGINAL_PROCESS_PAGE_CONTROL_END")

# ProcessPage OEM calls including context to establish _WriteData
# split points and mode-2/3 specific counter updates.
print("ORIGINAL_PROCESS_MODE23_WRITE_CALLS_BEGIN")
for i,(addr,line) in enumerate(asm):
    if 0x100007a74 <= addr < 0x10000889b and (
        "callq" in line or "job_size" in line
    ):
        print("OEM_WRITE_CONTEXT",hex(addr))
        for _, neighbor in asm[max(0,i-9):min(len(asm),i+3)]:
            print(neighbor)
print("ORIGINAL_PROCESS_MODE23_WRITE_CALLS_END")
