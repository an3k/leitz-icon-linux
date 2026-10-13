#!/usr/bin/env python3
"""Observe vendor classic raster2printer against a loopback-only TCP target.

No physical printer or remote host can be contacted: the test sets DEVICE_URI
to ipp://127.0.0.1 and binds only 127.0.0.1:9100. Source executable hash
verification is performed by the calling GitHub workflow.
"""
import hashlib
import os
from pathlib import Path
import socket
import subprocess
import sys
import threading

work, original, ppd = sys.argv[1:4]
work = Path(work)
data = bytearray()
events = []
shutdown = threading.Event()

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(("127.0.0.1", 9100))
server.listen(1)
server.settimeout(15)

def accept_one():
    try:
        client, peer = server.accept()
        events.append(f"CONNECTED={peer[0]}")
        client.settimeout(0.5)
        answered = False
        with client:
            while not shutdown.is_set():
                try:
                    fragment = client.recv(65536)
                except socket.timeout:
                    continue
                if not fragment:
                    break
                data.extend(fragment)
                if not answered and b"\x1bU" in data:
                    client.sendall(b"SKU:Ready;")
                    answered = True
                    events.append("SYNTHETIC_STATUS_REPLY=SKU:Ready;")
    except (OSError, socket.timeout) as exc:
        events.append(f"LOOPBACK_SERVER_ERROR={exc}")

thread = threading.Thread(target=accept_one, daemon=True)
thread.start()

env = os.environ.copy()
env["DEVICE_URI"] = "ipp://127.0.0.1/ipp/print"
env["PPD"] = ppd
options = "Resolution=300dpi PrintQuality=Draft OptimizationLevel=2 LabelCutter=ON PrintThroughPRN=OFF"
argv = [original, "101", "testuser", "Classic", "1", options,
        str(work / "input.ras")]
try:
    result = subprocess.run(argv, env=env, capture_output=True, timeout=18)
    code = result.returncode
    (work / "original-ipp-stderr.txt").write_bytes(result.stderr)
    (work / "original-ipp-stdout.bin").write_bytes(result.stdout)
except subprocess.TimeoutExpired:
    code = -999
    events.append("OEM_TIMEOUT=YES")
finally:
    shutdown.set()
    thread.join(timeout=2)
    server.close()

bytes_read = bytes(data)
(work / "original-ipp-loopback.bin").write_bytes(bytes_read)
print("OEM_LOCAL_IPP_EXIT=", code)
print("LOOPBACK_BYTES=", len(bytes_read))
print("LOOPBACK_SHA256=", hashlib.sha256(bytes_read).hexdigest())
print("LOOPBACK_PREFIX=", bytes_read[:64].hex(" "))
print("LOOPBACK_QUERY_ESC_U_COUNT=", bytes_read.count(b"\x1bU"))
print("\n".join(events))
print("LOOPBACK_ONLY=YES")
if code == -999 or not bytes_read:
    sys.exit(1)
