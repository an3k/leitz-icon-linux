#include "status_probe.h"

int leitz_win2018_is_queue_online(int opened,
                                   int info_retrieved,
                                   uint32_t printer_attributes)
{
    /* Original RVA 0x1520..0x15a9: OpenPrinterW, GetPrinterW level 2
     * twice; test PRINTER_INFO_2.Attributes bit 0x400 at x86 +0x34.
     * No actual USB probe and no examination of Status DWORD. */
    return opened && info_retrieved &&
           !(printer_attributes & LEITZ_WIN2018_PRINTER_ATTRIBUTE_WORK_OFFLINE);
}

int leitz_win2018_classify_version_probe(int write_succeeded,
                                          int read_succeeded,
                                          const uint8_t *reply,
                                          size_t received,
                                          int *boot_marker)
{
    /* OEM RVA 0x22a8..0x22e7 sends ESC V and reads a 64-byte
     * scratch buffer. At 0x22f1..0x230a strstr(reply,"BOOT")
     * optionally sets the output flag. At 0x2324 it returns
     * 1 whether or not the substring was present.
     * CanReadFromPrinter 0x1134..0x1141 checks that helper == 1.
     * This safe model does not emulate unbounded native strstr. */
    if (!boot_marker || received > LEITZ_WIN2018_PROBE_VERSION_MAX ||
        (received && !reply))
        return -1;
    if (!write_succeeded || !read_succeeded)
        return 0;

    size_t text_length = 0;
    while (text_length < received && reply[text_length] != 0)
        ++text_length;
    for (size_t i = 0; i + 4 <= text_length; ++i) {
        if (reply[i] == 'B' && reply[i+1] == 'O' &&
            reply[i+2] == 'O' && reply[i+3] == 'T') {
            *boot_marker = 1;
            break;
        }
    }
    return 1;
}
