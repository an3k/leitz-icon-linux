# Leitz Icon USB status and partial-job safety — evidence and implementation

This record distinguishes **original binary evidence**, **actual recorded USB traffic**, **current driver behavior**, and **untested hypotheses**. No new status command has been transmitted to the printer during this work.

## Status request/response: confirmed facts

| Request | Windows 2018 language monitor | Capture |
|---|---|---|
| `1B 55` (ESC U) | `leitz_oem_cartridge` | 7 actual requests/replies captured |
| `1B 41` (ESC A) | `leitz_oem_status` | No reply recorded in available captures |
| `1B 56` (ESC V) | `leitz_oem_version` | No reply recorded in available captures |

The existing user USB capture `leitz.pcapng` (not committed because it may contain user data) shows 7 ESC U requests and 7 USB IN responses. Two distinct responses, each **34 bytes including the trailing zero byte**, were observed:

- `SKU:70030001;INIT:11000;USED:273;\x00` (5 times)
- `SKU:70030001;INIT:11000;USED:282;\x00` (2 times)

This proves **ASCII key-value text terminated by NUL for this particular ESC U response on this device**; it does not establish the `INIT`/`USED` units or prove the response grammar for ESC A/V. The change in `USED` is a counter observation, **not** independent confirmation that a particular print finished.

Static `LeitzLm.dll` (amd64, driver version 2.17.4.112) disassembly confirms that the request mapping dispatches a two-byte command and uses an asynchronous read worker with a **0x400 (1024) byte receive buffer**, **0x1388 (5000 ms) wait**, and termination condition tied to requested length or a zero byte. That wait is specific to the examined Windows monitor function and must **not** be generalized to all device operations.

The current development backend and status command still **send only ESC U**. Their reads are now capped at 1024 bytes with a five-second shell timeout and their cartridge grammar is strictly checked. ESC A and ESC V are **not yet implemented** because response frames/values and safe coordination with printing remain unproven. For A/V, a read-only USB capture of those queries/answers is the next decisive primary evidence.

## Incomplete jobs and completion

- The original Windows `LeitzLm.dll` `EndDocPort` contains a `WaitForSingleObject` call with `0xea60` (60,000 ms); later calls `SetJobW` with control 7 (`JOB_CONTROL_LAST_PAGE_EJECTED`). Microsoft's documented operation marks a print job complete from the *language monitor*, **not** evidence of a specific printer-side acknowledgment.
- USB `write` success indicates transfer into the Linux USB stack; it does **not** prove that the printer advanced/cut the last label.
- An interrupted transfer may already have produced physical output. The backend **must never blindly replay** an uncertain/partial job. The backend currently exits 4 for write failures, i.e. CUPS `CUPS_BACKEND_STOP`, intended to stop the queue instead of CUPS automatically retrying the same job. The administrator should examine physical output before manually releasing/restarting a job.
- Observed source `04-Versand_FEHLERv2.pcapng` contains two nonempty USB OUT payloads, both ending in `ESC G ESC E ESC @ ESC Z`. Their presence proves only that those trailers were in captured host-side payloads; it does **not** establish what the printer actually completed. The basename is not error diagnosis.
- Do not append arbitrary 64-byte padding, automatically resend the final page, or claim a verified physical completion signal.
- Distinguish `PREPARATION_FAILED` (no bytes sent), `TRANSFER_STARTED_OR_PARTIAL` (printing uncertain), `TRANSMITTED_UNCONFIRMED` (all bytes accepted, hardware completion unverified), and future `DEVICE_CONFIRMED` only if an actual documented compatible device acknowledgment can be established.

CUPS backend exit statuses and error-policy semantics: https://apple.github.io/cups/doc/man-backend.html
Windows `JOB_CONTROL_LAST_PAGE_EJECTED`: https://learn.microsoft.com/en-us/windows/win32/printdocs/setjob

## USB access / ownership

The current device is exposed through Linux kernel `usblp` as `/dev/usb/lp0` (VID:PID `289e:1000`). The kernel `usblp_open()` permits only **one open descriptor at a time**, returning `EBUSY` on another attempted open. This works across Docker containers using the same kernel driver. Redundant `flock` files and `/run/leitz-icon` have therefore been **removed** from `leitzusb`, `leitz-status`, and the experimental probe. See [upstream `usblp.c`](https://github.com/torvalds/linux/blob/master/drivers/usb/class/usblp.c).

Limits: single-open enforcement is specific to the `usblp` path, **not** users that detach the kernel driver and use libusb directly. The device can continue moving or cutting after `close()`. A CUPS queue reporting `idle` does not prove the hardware has stopped.

The test probe is a separate opt-in program: one manufacturer status command, 1,024-byte bounded binary response, direct stdout hex. It does not generate spool data and creates no host or container capture files. The production print path remains unchanged apart from the removal of filesystem locks. See [one-shot status procedure](CONTROLLED_STATUS_CAPTURE.md).

**Validation state:** source and mocked-protocol checks; real A/V/P replies remain unverified. Only the development branch is changed; the running CUPS container on TARS and `main` are unchanged.

## Offline status-decoder implementation (development only)

Added `src/leitz_status.c/.h` to the existing `leitz-cli`. Command: `leitz-cli decode-status legacy|v3|lines|version saved-reply.bin`. This command reads **only regular files** (1..1024 bytes), does **not** open `/dev/usb/lpN`, and does **not** send any printer commands. It rejects oversized, empty and malformed replies, and preserves binary NUL bytes in ESC A/P snapshots.

- `legacy`: first-byte mapping from original macOS command filter. Unknown codes remain unknown.
- `v3`: maps the first byte conservatively; masked fault codes with overlapping active-low busy bit remain `FAULT_OR_BUSY_V3_AMBIGUOUS`, **not** an operational printer-state decision.
- `lines`: 1..4 big-endian bytes parsed as an **unverified candidate** count. Returns `COMPLETION_CONFIRMED=NO`; no continuation or resend is enabled.
- `version`: printable ASCII with optional terminal NUL (a conservative heuristic based on macOS text handling), not an established firmware-version wire grammar.

Offline verification: compiled strict C11 decoder/unit test locally and compiled the CLI with stubbed **unmodified codec dependencies**; exercised NUL-containing line counts, ambiguous status byte `C0`, version text, and invalid lengths. The full in-repository `make status-test` target was added for later end-to-end regression runs; **no physical status A/P/V reply has been captured yet**.

The production USB backend and `leitz-status` are **unchanged by this decoder step**. No automatic status probing or resume behavior has been activated, and neither `main` nor TARS was modified.
