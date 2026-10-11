# Controlled Leitz Icon USB status probe (development branch only)

The manually invoked `leitz-probe` sends **exactly one** status command to an operator-confirmed idle Leitz Icon and displays **both the raw reply bytes as hexadecimal and a decoded view on stdout**. It is never called by CUPS. No file, directory, host mount, or proprietary driver is required for captures.

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

For future testing, rebuild the separate experimental tag from the current development branch if its code has changed (does not replace the running `cups` container):

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

Example output for the **previously measured** `ESC A` reply (the decoded lines illustrate the new software; they were not emitted by the older probe binary):

```text
COMMAND=ESC A
REPLY_BYTES=1
REPLY_HEX=0x03
DECODED_STATUS_HEX=0x03
DECODED_STATUS_LEGACY=READY
DECODED_STATUS_V3=READY
```

For `ESC V`, output also includes `DECODED_TEXT`, `DECODED_NUL_TERMINATED`, and `DECODED_FIELD_ID/A/M/W` when those fields are present. The raw and decoded ID are printed **locally** but must not be pasted into a public issue or committed to the repository. Meanings of fields A/M/W remain unverified.

For a zero-byte timeout (as already measured for idle `ESC P`), the new probe prints `REPLY_BYTES=0` and `DECODED=NO_RESPONSE_WITHIN_TIMEOUT`, returns a non-zero exit code and does **not** fabricate a hex payload or printed-line count.

Review the **actual** output and the recorded results below before considering any further queries. This is one controlled sequence, not an automatic polling system. No extra host directories, bind mounts, status files or background services. Do not claim `ESC P` is safe during printing from an idle-only test.

The offline parser `leitz-cli decode-status legacy|v3|lines|version FILE` remains available for saved binary fixtures and does not access USB; it is **not** required merely to read the hex output.

## Verification

`make probe-test` compiles `leitz-probe` and exercises the binary transport with simulated duplex responses: zero bytes, fragments, timeout before a complete idle gap, oversized data, and unsupported commands. The formatter has separate offline unit tests for status, ASCII/NUL, structured fields, and candidate counters. This is not a printer hardware test. Actual idle replies from TARS are recorded below; tests for responses while printing have **not** been performed.

## First real-device results — 10 October 2026

Source: direct operator-provided output from TARS, Leitz Icon USB device `289e:1000`, kernel `usblp` path `/dev/usb/lp0`, using temporary `leitz-icon-cups:probe` container and `--idle-confirmed`. Operator confirmed CUPS idle/empty beforehand. **These were three separate manually invoked commands, not in-print queries.**

| Query | Actual result | Interpretation |
|---|---|---|
| `ESC A` (`1B 41`) | `REPLY_BYTES=1; reply byte 0x03` | Real device returned code `0x03`; the original macOS legacy/V3 decoder maps that byte to ready |
| `ESC V` (`1B 56`) | `REPLY_BYTES=34`, NUL-terminated printable ASCII `ID:[redacted];A:0129;M:0090;W:310c;;\x00` | Real response format confirmed; meanings of A/M/W not established; device ID deliberately redacted in this public repository |
| `ESC P` (`1B 50`) | `ERROR: Status exchange failed (-2), received 0 bytes; no retry` | Seven-second deadline elapsed with **no response bytes** while idle. Does not prove command unsupported during active printing or on other firmware |

**Implementation decision:** `ESC P` cannot currently be used to confirm print completion or resume an incomplete job. No automatic retries, no new in-print probes, no cutter/transport behavior change. `ESC A` READY does **not** confirm a finished mechanical cut; `ESC V` reports manufacturer fields without proven semantics. Do not extrapolate from one idle printer to all Leitz firmware variants.

The development image used on TARS was built from the branch as of that test; the running `cups` container, its printing pipeline, and `main` were not replaced by this probe.
