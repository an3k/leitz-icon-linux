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
