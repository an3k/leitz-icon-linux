# Controlled Leitz Icon USB status probe (development branch only)

The manually invoked `leitz-probe` sends **exactly one** status command to an operator-confirmed idle Leitz Icon and displays the **raw reply bytes as hexadecimal on stdout**. It is never called by CUPS. No file, directory, host mount, or proprietary driver is required for captures.

## Separation of responsibilities

- The existing `cups` container runs CUPS, its raster filter, and `leitzusb`; it must **not be replaced or restarted** merely to query printer status.
- The dedicated diagnostic runs as a **temporary container** from the experimental image, with the USB character device passed through. It never starts a separate CUPS daemon.
- Linux kernel `usblp_open()` rejects a simultaneous second open of `/dev/usb/lpN` with `EBUSY`. Consequently `flock` and `/run/leitz-icon` are unnecessary for the kernel-device access path. This guarantee does **not** apply to other USB access methods (e.g. a libusb user that detaches the kernel driver).
- Exclusive opening is **not** a printer-mechanical-idle check. A print backend may already have closed its descriptor while the printer is still moving or cutting.

## Constraints

- Explicit `--idle-confirmed` flag means that the operator has checked **both** an empty CUPS job queue and a printer that has finished all movement. It is not an automatic status gate.
- Exact character-device path `/dev/usb/lpN`, matching sysfs USB VID/PID `289e:1000`. Unsupported paths and devices are rejected before any USB query.
- `--command status` sends `1B 41` (ESC A); `version` sends `1B 56` (ESC V); `printed-lines` sends `1B 50` (ESC P). Each invocation sends one two-byte command, no raster, feed, cut, or recovery command.
- Binary read, maximum 1,024 bytes; **5-second deadline** for A/V and **7-second deadline** for P. The 1-second reply gap is a provisional capture heuristic, not a known manufacturer frame boundary. Timeout, oversized reply, open failure, or invalid path produces an error and **no successful status output**.
- No retries or automatic resume, and no claim that a successful USB reply establishes completed printing/cutting. `ESC P` while actively printing is **not authorized** by this procedure.

## Test procedure on TARS (only after explicitly choosing to capture)

The existing `leitz-icon-cups:probe` image was built from an **older commit** before the no-lock/hex-output rewrite. First rebuild that experimental tag from the current development branch (does not replace the running `cups` container):

```sh
docker build -f cups/Dockerfile -t leitz-icon-cups:probe \
  'https://github.com/an3k/leitz-icon-linux.git#port/vendor-parity'
```

Confirm no queued jobs and no physical printer movement:

```sh
docker exec cups lpstat -p LeitzIcon
docker exec cups lpstat -o LeitzIcon
```

Then **only when idle** run the single status inquiry:

```sh
docker run --rm --network none --read-only \
  --device /dev/usb/lp0:/dev/usb/lp0 \
  --entrypoint /usr/local/bin/leitz-probe \
  leitz-icon-cups:probe \
  --idle-confirmed --device /dev/usb/lp0 --command status
```

Expected output structure (illustration only; **not a measured response**):

```text
COMMAND=ESC A
REPLY_BYTES=<actual length>
REPLY_HEX=<actual bytes in hexadecimal>
```

Review that **actual** output before scheduling `--command version` or `--command printed-lines`. This is one controlled sequence, not an automatic polling system. No extra host directories, bind mounts, status files or background services. Do not claim `ESC P` is safe during printing from an idle-only test.

The offline parser `leitz-cli decode-status legacy|v3|lines|version FILE` remains available for saved binary fixtures and does not access USB; it is **not** required merely to read the hex output.

## Verification

`make probe-test` compiles `leitz-probe` and exercises the binary transport with simulated duplex responses: zero bytes, fragments, timeout before a complete idle gap, oversized data, and unsupported commands. This is not a printer hardware test. The image build and actual `ESC A` response must be verified again on TARS after the branch updates.
