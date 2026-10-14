#!/usr/bin/env python3
"""Compare the OEM and reconstructed macOS CUPS raster executables with
the *same loopback-only IPP URI*. This cannot address an external printer.

The genuine OEM path can use stdout rather than TCP for a CUPS raster job,
even when DEVICE_URI is ipp://127.0.0.1. Do not invent a socket connection:
compare what each actual executable emits on every channel.
"""
import os
from pathlib import Path
import socket
import subprocess
import sys
import threading

work, vendor, ppd = sys.argv[1:4]
work = Path(work)
candidate = ("reconstructions/macos-5.1.465-classic/raster2printer/"
             "original_raster2printer_cups_candidate")
arguments = [
    "705", "tester", "IPP", "1",
    "Resolution=300dpi PrintQuality=Draft OptimizationLevel=2 "
    "LabelCutter=ON PrintThroughPRN=OFF DebugMode=Release",
    str(work / "input.ras"),
]


def run_one(binary, label):
    # Listen only on 127.0.0.1. Each executable gets a fresh listener,
    # preventing state carryover or an accidental remote connection.
    receiver = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    receiver.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    receiver.bind(("127.0.0.1", 9100))
    receiver.listen(1)
    receiver.settimeout(0.7)
    network_data = bytearray()
    faults = []
    finish = threading.Event()

    def accept():
        try:
            connection, _ = receiver.accept()
            connection.settimeout(0.4)
            with connection:
                while not finish.is_set():
                    try:
                        packet = connection.recv(65536)
                    except socket.timeout:
                        continue
                    if not packet:
                        break
                    network_data.extend(packet)
                    if b"\x1bU" in network_data:
                        connection.sendall(b"SKU:Ready;")
        except socket.timeout:
            pass
        except OSError as exc:
            faults.append(str(exc))

    thread = threading.Thread(target=accept, daemon=True)
    thread.start()
    env = os.environ.copy()
    env["DEVICE_URI"] = "ipp://127.0.0.1/ipp/print"
    env["PPD"] = ppd
    try:
        result = subprocess.run([binary, *arguments], env=env,
                                capture_output=True, timeout=18, check=False)
    finally:
        finish.set()
        thread.join(timeout=1.5)
        receiver.close()

    (work / ("ipp-" + label + ".out")).write_bytes(result.stdout)
    (work / ("ipp-" + label + ".err")).write_bytes(result.stderr)
    (work / ("ipp-" + label + ".tcp")).write_bytes(network_data)
    if faults:
        raise RuntimeError(f"{label}: loopback receiver error: {faults}")
    return result.returncode, result.stdout, result.stderr, bytes(network_data)


if not Path(vendor).is_file() or not Path(candidate).is_file():
    raise SystemExit("OEM_CONNECTED_IPP_PARITY=UNTESTED: binary missing")

v = run_one(vendor, "vendor")
c = run_one(candidate, "joined")
route = "TCP" if v[3] else "STDOUT" if v[1] else "NONE"
if v == c and (v[1] or v[3]):
    print("OEM_CONNECTED_IPP_PARITY=YES "
          f"ROUTE={route} EXIT={v[0]} STDOUT_BYTES={len(v[1])} "
          f"TCP_BYTES={len(v[3])} STDERR_BYTES={len(v[2])}")
else:
    print("OEM_CONNECTED_IPP_PARITY=NO "
          f"OEM_EXIT={v[0]} C_EXIT={c[0]} "
          f"OEM_STDOUT={len(v[1])} C_STDOUT={len(c[1])} "
          f"OEM_TCP={len(v[3])} C_TCP={len(c[3])}")
    for index, channel in enumerate(("exit", "stdout", "stderr", "tcp")):
        if v[index] != c[index]:
            print("DIFFERENT_CHANNEL=", channel)
            if index in (1, 3):
                lhs, rhs = v[index], c[index]
                mismatch = next((i for i in range(min(len(lhs), len(rhs)))
                                 if lhs[i] != rhs[i]), min(len(lhs), len(rhs)))
                print("FIRST_DIFFERENT_OFFSET=", mismatch)
                print("OEM_CONTEXT=", lhs[max(0, mismatch-16):mismatch+64].hex())
                print("C_CONTEXT=", rhs[max(0, mismatch-16):mismatch+64].hex())
            if index == 2:
                import difflib
                a = v[index].decode("utf-8", "replace").splitlines()
                b = c[index].decode("utf-8", "replace").splitlines()
                print("\n".join(list(difflib.unified_diff(
                    a, b, fromfile="OEM", tofile="joined"))[:90]))
    sys.exit(1)
