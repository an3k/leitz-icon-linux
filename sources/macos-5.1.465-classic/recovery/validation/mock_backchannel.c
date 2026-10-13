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
