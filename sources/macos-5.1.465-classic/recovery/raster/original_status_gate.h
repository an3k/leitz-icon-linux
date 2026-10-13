#ifndef CLASSIC_ORIGINAL_STATUS_GATE_H
#define CLASSIC_ORIGINAL_STATUS_GATE_H
/* Active entry decision 0x1000091c8..0x100009243 of original
 * macOS 5.1.465 _CheckPrinterStatus, independent of the status parser.
 * This returns only which observed block is entered, not the OEM's
 * complete function return value or ABI. */
int classic_original_status_uses_resolver(const char *uri,
                                           int socket_fd,
                                           int terminate_flag);
#endif
