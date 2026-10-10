# Controlled Leitz Icon status captures (development branch only)

**Purpose:** record actual, binary-safe USB replies to the already identified original manufacturer queries without changing the CUPS printing path. **Not a printer-side completion protocol**.

## Status and limits

- Experimental binary `leitz-probe` is built into the development Docker image but is **never called by CUPS, startup scripts, or status polling**. Requires explicit operator invocation.
- Exactly **one** USB query per invocation: `--command status` → `ESC A`, `version` → `ESC V`, or `printed-lines` → `ESC P`. No `ESC G`, `ESC E`, cutter, media feed, or print raster data sent.
- Requires `--idle-confirmed` and a separately verified **mechanically stopped**, not printing/cutting printer. This flag records **operator confirmation**, not an automated idle test. Do **not** run it while a CUPS print is active or the device is moving.
- Requires root in the isolated CUPS container; strictly allows `/dev/usb/lpN`, verifies exact USB vendor 289e/product 1000 in sysfs, and rejects symlink device paths.
- Takes the same per-port advisory `flock` lock `/run/leitz-icon/lpN.lock` as `leitzusb` and `leitz-status`. Busy device/failed lock → abort without query. No other container or tool is protected unless it uses the same shared lock and filesystem.
- Opens USB nonblocking, sends a single two-byte query, and receives raw bytes using `poll`. Deadline **5 seconds for A/V**, **7 seconds for P**. A **1-second response-idle gap** is used because actual reply framing is not yet established; this is a capture heuristic, **not** a protocol specification. If the overall timeout expires before the idle gap, the capture is rejected as incomplete.
- Maximum **1,024 bytes**. A 1,025th byte causes rejection. NUL bytes and fragment boundaries are preserved in the captured payload; no Bash string processing or human-readable assumptions.
- Captures go into a caller-specified **new file** (`O_CREAT|O_EXCL|O_NOFOLLOW`, mode 0600); existing files are never overwritten. A failed capture removes the partial output.
- **No retries**, no automatic ESC P during an active print, no resume, no job completion claims. Only the existing `leitz-cli decode-status` can interpret saved snapshots offline.

## Future one-time physical test — do not run until scheduled

Do not build/deploy this branch to TARS merely to run the probe; include it with the planned batched driver hardware testing. Once the development image is explicitly built/installed, the CUPS queue is idle, and all mechanical movement has stopped:

```sh
# Example for a container named cups, verified device /dev/usb/lp0:
docker exec -u 0 cups /usr/local/bin/leitz-probe \
  --idle-confirmed --device /dev/usb/lp0 --command status --output /tmp/leitz-A.bin
docker cp cups:/tmp/leitz-A.bin ./leitz-A.bin
```

Review that first raw capture and device behavior before choosing whether to run `version` or `printed-lines`. Use distinct output names `/tmp/leitz-V.bin` and `/tmp/leitz-P-idle.bin`. Only use `ESC P` during printing if a **separate explicitly approved test** establishes that doing so is safe and necessary. Avoid committing raw USB data containing unknown private metadata to a public repository.

For offline analysis:
```sh
leitz-cli decode-status legacy leitz-A.bin
leitz-cli decode-status v3 leitz-A.bin
leitz-cli decode-status version leitz-V.bin
leitz-cli decode-status lines leitz-P-idle.bin
```

These decoders yield provisional interpretations. A valid-looking `ESC A` byte or `ESC P` counter does not independently confirm physical printing/cutting completion.

## Offline verification

`make probe-test` compiles the recorder and runs a simulated full-duplex socket-based protocol test. It checks raw `0x00`, split replies, timeouts, unexpected oversized replies, and unsupported commands. This is **not** a USB device test. The recorder also fails closed on a machine without a matching USB device. Full CUPS image and live hardware behavior remain untested at this step.
