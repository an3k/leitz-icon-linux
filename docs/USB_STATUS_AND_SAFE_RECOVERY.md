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

Previously `cups/leitzusb` and `cups/leitz-status` both opened `/dev/usb/lpN` without a mutual-exclusion mechanism. Status requests could race the print process and consume the wrong response.

Development branch now adds one lock for each `lpN` device, in `/run/leitz-icon/lpN.lock`:
- Print backend: acquire exclusive lock with up to 30 seconds' wait, *before opening USB and ESC U preflight*, hold through the transfer.
- Standalone status: fail immediately with `STATE=BUSY` while lock is owned by the print backend.
- Both use `flock`; the runtime image explicitly installs `util-linux`.
- The `/run/leitz-icon` lock directory is owned by the container (normally running device operations as root) with permissions 0700.
- This **advisory** lock only protects processes using the same file/inode in the same filesystem namespace. Multiple containers need a *shared lock directory* and must not independently access the device. Direct other programs using `usblp` **do not** honor our lock. Ideally the CUPS container is sole owner and all other clients use its status/printing interface.
- Lock release follows the end of the USB **transfer**, **not** verified end of the mechanical print operation. Consequently status access after the last write is not yet a reliable `printer idle` proof. Avoid initiating additional arbitrary status queries until reply semantics and safe operating phases have been established.

**Validation state:** static source checks and byte-level review; no hardware qualification of new lock/read behavior. TARS and `main` unchanged.
