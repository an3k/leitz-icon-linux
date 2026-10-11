# Leitz Icon Linux — development evidence and status (11 October 2026)

> This document preserves the historical project record in English.
> It is not a declaration of hardware release readiness. Earlier paths and
> branch names below are explicitly historical; the current active branches
> are `main` (approved baseline), `driver` (standalone driver),
> `tests` (independent verification) and `cups` (frozen CUPS base).

## Architecture and source inventory

The shared CUPS raster and job controller is protocol-neutral.
The Current and Native Linux 1.0.3 encoders and binary validators are built
as independent shared libraries, `leitz-current.so` and `leitz-native.so`.
The runtime loader checks the ABI and resolves a module for each newly
started print filter. `LeitzProtocol=Auto` prefers Current and falls back to
Native. The CUPS PPD does not change when a protocol module is removed.
The USB backend validates an entire job before sending anything to USB.
The previous backend defect that referenced an unset dialect variable was
subsequently corrected. The hardware qualification of this modular code
remains outstanding.

The authoritative development media catalog has **86 PPD profiles**:
43 A profiles and 43 `Leitz`-prefixed B profiles, comprising 64 fixed-length
and 22 continuous profiles. The generator produces the CUPS PPD and media
header, preserving 344 PPD media definitions (four per profile).
These entries do not imply 86 distinct physical media. Four A/B geometries
are intentionally not identical:
`LargePendaflexTabRotated`, `Multipurpose70030001Rotated`,
`NameBadgeCard70190001Rotated`, `ShippingLabel70180001Rotated`.
Of the B display names, 39 originated from Windows resources and four
were derived. The older `tools/media_b_original_labels.json` was a
historical intermediate source and is no longer part of the driver.

Original vendor catalogs independently contained **40** Linux 1.0.3 PPD
formats, **41** macOS 1.2.190 PPD formats and **76** Windows GPD options
(75 fixed plus `CUSTOMSIZE`). The complete original vendor archives
are not committed here. Their examined behavior is recorded in
`PORTING_STATUS.md` and `VENDOR_LINUX_PROTOCOL.md`.

Physical cassette/SKU, physical stock width, CUPS page selection and the
printable raster area remain distinct properties. The previously used
170 mm value was a test length, not a general driver limit. Continuous
materials need job-dependent length and cutting behavior.

## Hardware observations versus offline implementation

| Feature | Current evidence | Remaining verification |
| --- | --- | --- |
| White 88 mm continuous paper | Historical Current encoder printed successfully at 300×300 and 300×600 dpi, including variable lengths, multiple labels and automatic cuts | Re-qualify the modular driver and native encoder |
| Yellow 88 mm plastic | A 60 mm request produced approximately 49–51 mm; a subsequent 100 mm request stalled partway through the first printed row | Cause unknown; investigate as one controlled batch |
| ESC U cartridge query | Physical ASCII/SKU replies captured | Other cartridge SKUs and capacity semantics unverified |
| ESC A ready query | One physical idle response, byte `0x03`, and original binary decoder evidence | Do not infer successful mechanical job completion |
| ESC V manual version query | 34-byte NUL-terminated textual response observed | Exact contents and firmware behavior not established |
| ESC P printed-lines query | Idle request yielded zero bytes and timed out after seven seconds | Device progress and safe recovery not established |
| CUPS ReportStatus | Offline implementation via restricted command filter and backend | Live behavior not physically verified |
| Cutter ON/OFF | Shared multipage policy implemented in Current and Native paths; equal width and resolution per job, variable page length permitted | Native and revised multi-page cutting not physically verified |
| Grayscale conversion | 8-bit input, selectable Bayer4 dithering/threshold; offline regressions | No proven equivalence to original macOS rendering algorithms |
| Media profiles | 86 profiles structurally validated | Most cassettes, die-cut materials and four A/B geometry differences not physically tested |

An `ESC U` response can show a changing usage counter, but this is not
independent proof that a particular label finished printing.
Completion of USB writes also does not prove printing or cutting has ended.
Any uncertain partially delivered print job must not be retried blindly.

## Remaining work (ordered priority)

1. Confirm printer motion, physical job completion, and cutting after transfer;
   determine safe during-print status/progress semantics. USB write completion
   alone is insufficient.
2. Determine and correct yellow-cassette geometry/partial-job failures based
   on captured device behavior. Do not introduce arbitrary fixed-length limits.
3. **Highest-priority software porting:** compare original Linux 1.0.3
   behavior against Current; transfer evidence-backed features to **Current
   only**. Freeze the existing original-derived Native module unchanged.
   FullWidthRaw and TrimmedRaw uncompressed raster options have been added
   to Current while preserving its established optimized default output.
   Other source optimization modes and protocol differences remain unported
   until supported by verified evidence.
4. Qualify the modular development driver on real hardware: Current and
   unchanged Native, 300x300/300x600, variable lengths, multi-page jobs
   and cutter ON/OFF; test new Current raw option separately.
5. Verify other cassette SKUs, die-cut stock, actual printable widths,
   remaining media formats and print regions.
6. Complete live status, progress, error handling and safe recovery; compare
   macOS/Windows differences and the conditional 64-byte behavior without
   speculative padding.
7. Finish end-to-end CUPS/Unraid integration, deployment and packaging;
   offline CI/mock tests are not mechanical completion proof.
8. Select the redistribution license before a public release.

## Historical development sequence (not the current repository layout)

Earlier development took place on `port/vendor-parity`, with `main`
retained as a comparison baseline. A previous snapshot placed
`main` at `1afdab16` and `port/vendor-parity` at `2fb8a9d9`,
with the latter 187 commits ahead at that time. GitHub Actions run
`38125541659` passed the contemporaneous offline build, media, raster,
status and Docker checks. Those historic results were not real printer tests.

An earlier modular design stored `leitz_cups_filter.c`,
`leitz_job.c`, `leitz_protocol_current.c`,
`leitz_protocol_native_linux_103.c`, `leitz_protocol.h` and
`leitz_protocol_registry.c` directly under `src/`. The controller
managed raster, page sequence, feed and cutters for both encoders.
Selecting `LabelCutter=OFF` was moved out of the Native-specific
path into shared job control. The original Current single-label output
was preserved while multi-label no-cut paths remained experimental.
Standalone reference generators and validators were used to compare
raw framing at 300×300 and 300×600 dpi.

Older media aliases added to reach the 43+43 profile arrangement included
`LeverArchFile1005`, `LeitzContinuous112`, `LeitzDHLShippingLabel`,
`LeitzNameBadgeCard8214`, and `LeitzWristband`. Historic test scripts
used `make mock-filter`, `make media-check`, and
`tests/test_v04.py`. These names are retained as a record, not as
commands for the current source layout.

The obsolete mixed branch `port/vendor-parity` was fully inventoried
and deleted only after its driver files, tests and research documentation
had been preserved in dedicated branches. No claim of a mechanically
completed print was inferred from that change.

## Planned verification order

First complete related offline source and CI checks. Then carry out one
coordinated hardware session covering white/yellow stock, both encoders,
fixed and variable label lengths, multipage jobs, both resolutions and
both cutter modes. Only after the data supports a specific protocol and
device behavior should it be promoted to a production release.
