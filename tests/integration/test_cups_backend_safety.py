#!/usr/bin/env python3
"""Offline CUPS backend safety contract; never opens a USB device."""
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]
backend = ROOT / "driver/cups/leitzusb"
entry = ROOT / "driver/cups/entrypoint.sh"

for file in (backend, entry):
    checked = subprocess.run(["bash", "-n", str(file)], capture_output=True, text=True)
    assert checked.returncode == 0, (file, checked.stderr)

wire = backend.read_text(encoding="utf-8")
cups = entry.read_text(encoding="utf-8")

# A signal during transfer may leave a printed fragment. CUPS_BACKEND_FAILED
# (1) permits policy-driven replay; CANCEL (5) is final for this job.
cancel = re.search(r"(?ms)^on_cancel\(\)\s*\{(.*?)^\}", wire)
assert cancel, "Missing cancellation handler"
assert re.search(r"(?m)^\s*exit 5\s*$", cancel.group(1)), "Cancellation must not use retryable status"

# Prevent reproducing the prior successful-transfer failure: set -u was on,
# but dialect was used without ever having been assigned.
assert "set -euo pipefail" in wire
assert 'validate-auto "$tmp"' in wire
assert "mapfile -t validation_fields" in wire
assert 'PROTOCOL=Current) dialect=Current' in wire
assert 'PROTOCOL=Native)  dialect=Native' in wire
assert "INFO: Leitz $dialect job transferred to USB" in wire

# Normal CUPS error policy may replay failed filters/backend jobs. Explicit
# stopping for newly created queues keeps uncertain output from being retried.
assert "-o printer-error-policy=stop-printer" in cups
assert "lpadmin -p LeitzIcon -E" in cups
print("PASS cancellation is final, validation reports protocol, and queue errors cannot auto-replay")
