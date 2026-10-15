/* Interposes the original macOS CUPS backchannel function only within
 * disposable CI processes. Never opens or transmits to a real printer.
 * This code is a test fixture, not reconstructed manufacturer source.
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern ssize_t cupsBackChannelRead(char *, int, double);

static ssize_t substitute_read(char *buffer, int capacity, double deadline) {
    const char *setting=getenv("LEITZ_MOCK_REPLY");
    (void)deadline;
    if (!setting || !strcmp(setting,"NONE")) {
        fputs("MOCK_BACKCHANNEL: NONE\n",stderr);
        return 0;
    }
    if (!strcmp(setting,"READ_ERROR")) {
        fputs("MOCK_BACKCHANNEL: ERROR\n",stderr);
        return -1;
    }
    if (!strcmp(setting,"READY")) {
        if (capacity<1) return -1;
        buffer[0]=3;
        fputs("MOCK_BACKCHANNEL: READY\n",stderr);
        return 1;
    }
    /* Additional OEM ESC A state bytes: empty roll, jam, general error,
     * and an unrecognized status. The underlying byte mapping is taken
     * from the disassembled original's exact comparisons. */
    if (!strcmp(setting,"MEDIA_EMPTY") || !strcmp(setting,"MEDIA_JAM") ||
        !strcmp(setting,"GENERAL_ERROR") || !strcmp(setting,"UNKNOWN_STATUS")) {
        if (capacity<1) return -1;
        buffer[0]=!strcmp(setting,"MEDIA_EMPTY") ? 0x20 :
                  !strcmp(setting,"MEDIA_JAM") ? 0x30 :
                  !strcmp(setting,"GENERAL_ERROR") ? 0x40 : 0x23;
        fprintf(stderr,"MOCK_BACKCHANNEL: STATUS 0x%02x\n",
                (unsigned char)buffer[0]);
        return 1;
    }
    /* Largest OEM response with a defined in-bounds NUL terminator:
     * GetStatus allocates 1024 bytes then stores reply[received]=0.
     * Deliberately do not inject 1024 (OEM would write out of bounds).
     * The same synthetic backchannel is used for OEM and recovered code. */
    if (!strcmp(setting,"LONG_1023")) {
        if (capacity < 1023) return -1;
        memset(buffer,'X',1023);
        fputs("MOCK_BACKCHANNEL: 1023 bytes\n",stderr);
        return 1023;
    }
    size_t n=strlen(setting);
    if (capacity<0 || n>(size_t)capacity) return -1;
    memcpy(buffer,setting,n);
    fprintf(stderr,"MOCK_BACKCHANNEL: %zu bytes\n",n);
    return (ssize_t)n;
}

__attribute__((used,section("__DATA,__interpose")))
static const struct {
    const void *replacement;
    const void *original;
} substitution = {
    (const void *)substitute_read,
    (const void *)cupsBackChannelRead
};
