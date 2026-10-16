# Windows 2018 HTTP WLAN interface — partial reconstruction

Extracted from the **original** Windows \`LeitzIconAddPrinterTool.exe\`,
SHA-256 \`e27663457368f529cc64322a7550431ec2ff6b28e4d640a87419b837df924435\`.

The managed application has a **separate control-panel HTTP** WLAN
configuration path: \`Scan.xml?scan=1\`, \`ScanRow.xml?ScanRow=<index>\`,
and \`POST Main.html\` using its documented
\`application/x-www-form-urlencoded\` form field names/order.

\`wifi_http.[ch]\` reconstructs exact relative scan URIs and the
manufacturer's literal POST field layout offline. Input byte percent
encoding is **deliberately standards-compliant** rather than a claim
that the original application escapes every input character likewise.
The C API does NOT authenticate, contact a printer, send Wi-Fi
secrets, establish wireless connections, modify firmware, or implement
the manufacturer's original .NET ABI.

Never log or persist the resulting secret-bearing POST body. A secure
zeroization function is provided. The original XML response examples
and authentication/session semantics have not yet been recovered.

Compile/test on Linux:
\`gcc -std=c11 -Wall -Wextra -Werror -pedantic -O2 wifi_http.c wifi_http_test.c -o /tmp/wifi-http && /tmp/wifi-http\`


## Recovered AddPrinter managed scan retry, security modes and original ASCII form

**No duplicate work:** sources HEAD 4c88c09d489d7ec73b37ff6302cd0c60e64ce4a3
verified before writes. The existing \`wifi_http.[ch]\` remains
standards-oriented; this **separate** \`wifi_managed.[ch]\` captures the
distinct original Microsoft .NET application's behavior, not
a modification of the already passing implementation.

Primary binary: Windows 2018 \`LeitzIconAddPrinterTool.exe\`
SHA-256 e27663457368f529cc64322a7550431ec2ff6b28e4d640a87419b837df924435.
Read PE/CLI metadata streams #~, #Strings, #US and MethodDef IL
**offline**, without executing the application or making HTTP requests.

- \`GetWiFiScanInfo\` MethodDef 267, RVA 0x6548, IL SHA-256
  308b5b972e83576255c219e763de99534102adc1127c108a97060c9ba33066a7:
  first requests \`Scan.xml?scan=1\`, parses first response node;
  if \`ScanInProgress\` is not \`"1"\` (case-sensitive String.Equals,
  StringComparison.Ordinal=3), reads \`NumScanResults\` and \`MySsid\`.
  Otherwise polls \`Scan.xml\` immediately and then with 300ms sleeps
  while in progress, limited to 50 follow-up retrievals. Missing or
  empty response yields null, and caught exceptions are logged.
  No invented unlimited polling or synchronous background tasks.
- \`GetWiFiNetworkInfo\` MethodDef 269, RVA 0x67e4, IL SHA-256
  ebb1583d45a94d6fe1330f9a0f5e11d2d48995f867ad4daab4409ec0ae255993:
  selects nodes via \`response/bss\`, inspects the first child
  node's \`Valid\`, \`Name\`, \`SecurityMode\`, \`NetworkType\`
  and \`Strength\` with SelectSingleNode and get_InnerText;
  compares Valid with "1" using StringComparison.Ordinal.
  The latter three numeric values are parsed from XML text
  by the application's managed Integer.Parse method.
  No actual hardware XML response was present in the upload;
  only selector identities and control flow are verified.
- \`GetSecurityModeString\` MethodDef 271, RVA 0x6a88, 112 IL bytes:
  mode 0 => None, mode 1 => WEP, with password length 5/10 -> WEP40,
  13/26 -> WEP104; modes 5,9,13 -> WPA; other modes -> None.
  These are original application numeric **enum values**, not
  invented firmware capability guarantees.
- \`SetWiFiNetwork\` MethodDef 270, RVA 0x6960, IL SHA-256
  9921566b368276b37b6432256ba75c3980f0512a85d5ea103b092cdd86df035c:
  fetches \`Main.html\`, sets credentials, constructs the literal
  form with String.Format, replaces \`&\` with \`%26\` on
  **only the security key**, not on SSID or mode. It obtains
  \`System.Text.Encoding.Default\`, converts the formatted body
  into bytes and sends POST with \`application/x-www-form-urlencoded\`.
  There is **no evidence** the OEM percent-escapes every input value.
  Earlier generic RFC 3986 encoder therefore must not be mistaken
  for original byte parity. It also uses credentials inherited from
  the WebRequest, which have not been independently reconstructed.

\`wifi_managed.[ch]\` includes:
1. Exact IL-derived numeric mode/key-length mapping.
2. OEM-compatible ASCII form builder (raw SSID/mode; only
   ampersands in key replaced by %26). Non-ASCII is explicitly
   excluded because Windows \`Encoding.Default\` depends on its
   configured legacy codepage.
3. Pure finite scan progress state with an initial fetch and
   at most 50 follow-ups; delay and actual HTTP remain outside
   this offline C API. A final 300ms sleep before OEM time-out
   is described, not automatically executed.

Strict GCC and Clang tests cover modes, WEP lengths, byte-for-byte
ASCII form including unescaped SSID '&', insufficient buffers,
non-ASCII rejection, first scan, polling exhaustion, and immediate
completion. No network configuration, device requests, personal
Wi-Fi credentials, firmware changes or productive Linux code.


## OEM AddPrinter managed scan and BSS XML response selection

**Pre-write check:** sources HEAD a11679f61568ac948dfe6ead514f482d6bdbb0a8.
Previous managed GetWiFiScanInfo retry (50 attempts, 300 ms),
GetWiFiNetworkInfo selectors, original security mapping,
ASCII form-encoding implementation and all existing tests already
present. No existing source routines were overwritten.

**Primary evidence:** user-supplied original Windows 2018
LeitzIconAddPrinterTool.exe, SHA-256
e27663457368f529cc64322a7550431ec2ff6b28e4d640a87419b837df924435.
.NET IL MethodDef 267 RVA 0x6548 uses the literal child names
ScanInProgress, NumScanResults, MySsid. MethodDef 269 RVA 0x67e4
selects response/bss, its first child, then literal child
names Valid, Name, SecurityMode, NetworkType, Strength.
The original compares progress/valid with the **exact** string
"1" (StringComparison.Ordinal), and parses the three latter
BSS numeric texts as signed Int32 values. Actual OEM responses,
XML schemas, namespace conventions and missing-node semantics
are not available in the user upload.

**New offline code:** wifi_xml.[ch] models only those SELECTED
XML fields in portable C using Expat. The fixture envelope
\`<response>...\` and row wrapper \`<response><bss><row>...\`
are **inferred** from the managed selector paths; no claim of
actual response-schema parity. XML is decoded with entity
handling, rejects DTD, does not fetch network resources,
bounds input to 64 KiB, individual field text to 255 bytes,
and rejects repeated known fields, invalid integers,
malformed XML and over-nesting. It treats in-progress scans
as having no result count and invalid BSS rows as having no
detail values. All original source-code modifications are
standalone in sources, not the production Linux driver.

**Offline verification:** GCC and Clang -std=c11 -Wall -Wextra
-Werror -pedantic -O2, with local AddressSanitizer and
UndefinedBehaviorSanitizer runs, passed. Synthetic cases
include XML entity decoding, exact "1" versus whitespace-
padded text, signed fields, missing/invalid/overflow integer
fields, DTD injection, repeated field, truncated document,
unknown fields and unchanged caller output after errors.
The GitHub Windows-only workflow separately compiles/runs
the same C tests with system Expat. No real HTTP request,
printer reply capture, credentials, COM calls, USB writes
or firmware operations were performed.

**Still open:** OEM-authenticated HTTP response examples,
exact XML root/wrapping/error semantics, retrieval and
authentication handling, true OEM application execution
comparison and physical device tests. Original managed scan
and security-mode policies were not duplicated or modified.


## Original Windows AddPrinterTool WiFi password validation patterns

Pre-write verification: branch sources HEAD
70d6203592e101b436cfd0de40a2781d27b91d89;
the prior Windows managed scan/HTTP form, XML replies and raster
reconstructions already exist and are left unchanged.

Primary source: the user-provided Windows 2018 installer
LeitzIconAddPrinterTool.exe (SHA-256
e27663457368f529cc64322a7550431ec2ff6b28e4d640a87419b837df924435).
Its managed MethodDef 285 CheckWiFiPasswordPattern, RVA 0x6c34,
contains direct calls to security-code getter (MethodDef 292)
and Regex.IsMatch. Its static initializer MethodDef 286,
RVA 0x6d18, builds **five** Regex instances with exact literals:
- ^[0-9a-fA-F]{10}$
- ^.{5}$
- ^[0-9a-fA-F]{26}$
- ^.{13}$
- ^.{8,63}$

IL branch reconstruction:
- mode 0: returns TRUE before checking password;
- other modes: null/empty returns FALSE and resource key
  AddPrinterToolEnterPassword;
- mode == 1: accepts ANY of first four patterns above,
  otherwise resource key AddPrinterToolPasswordIncorrect;
- mode > 1 (signed comparison): requires final ^.{8,63}$
  match, otherwise incorrect-password resource;
- a negative numeric mode with nonempty password returns TRUE.
This differs from the separately reconstructed method
GetSecurityModeString (MethodDef 271): that mapping calls only
5, 9, 13 "WPA", while password validation applies 8..63
to *all* mode codes > 1. Never silently combine those decisions.

New portable, standalone C wifi_password.c/.h and test use
UTF-16 code-unit input to match .NET code-unit counts and support
the original Regex '$' before trailing LF, with explicit
input bounds. They do not store, log, send or modify the secret.
New error return for unterminated input is a safety extension.
No claim of identical .NET regex exceptions, case-folding for
other scripts, UI localization, printer security support,
managed-runtime parity or hardware authentication.

Locally tested with GCC and Clang -std=c11 -Wall -Wextra
-Werror -pedantic -O2, AddressSanitizer and
UndefinedBehaviorSanitizer; CI runs the same deterministic
functional cases without sanitizer dependencies.
