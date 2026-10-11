/* Offline output tests: no USB device, Docker, or live printer needed. */
#include "../../src/leitz_probe_format.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *format(uint8_t cmd, const uint8_t *data, size_t length) {
    FILE *f=tmpfile();
    assert(f);
    assert(leitz_probe_format(f,cmd,data,length)==0);
    assert(fseek(f,0,SEEK_END)==0);
    long size=ftell(f);
    assert(size>=0 && size<16384);
    assert(fseek(f,0,SEEK_SET)==0);
    char *out=calloc((size_t)size+1,1);
    assert(out && fread(out,1,(size_t)size,f)==(size_t)size);
    assert(fclose(f)==0);
    return out;
}

static void contains(const char *text, const char *value) {
    if (!strstr(text,value)) {
        fprintf(stderr,"Missing '%s' in:\n%s\n",value,text);
        abort();
    }
}

int main(void) {
    const uint8_t ready[]={0x03};
    char *out=format('A',ready,sizeof ready);
    /* Output contract: exact field labels/values; no invented status data. */
    const char *expected_ready =
        "COMMAND=ESC A\n"
        "REPLY_BYTES=1\n"
        "REPLY_HEX=0x03\n"
        "DECODED_STATUS_HEX=0x03\n"
        "DECODED_STATUS_LEGACY=READY\n"
        "DECODED_STATUS_V3=READY\n";
    assert(strcmp(out,expected_ready)==0);
    contains(out,"COMMAND=ESC A\nREPLY_BYTES=1\nREPLY_HEX=0x03\n");
    contains(out,"DECODED_STATUS_HEX=0x03\n");
    contains(out,"DECODED_STATUS_LEGACY=READY\n");
    contains(out,"DECODED_STATUS_V3=READY\n");
    assert(strstr(out,"READY_LEGACY")==NULL && strstr(out,"UNVERIFIED")==NULL);
    free(out);

    const uint8_t unknown[]={0xff,0x55};
    out=format('A',unknown,sizeof unknown);
    contains(out,"REPLY_HEX=0xFF 0x55\n");
    contains(out,"DECODED_STATUS_LEGACY=UNKNOWN\n");
    contains(out,"DECODED_EXTRA_STATUS_BYTES=UNINTERPRETED\n");
    free(out);

    /* Synthetic 34-byte format; does not disclose an actual device ID. */
    const uint8_t version[]="ID:7654321;A:0129;M:0090;W:310c;;";
    out=format('V',version,sizeof version);
    contains(out,"COMMAND=ESC V\nREPLY_BYTES=34\n");
    contains(out,"DECODED_TEXT=ID:7654321;A:0129;M:0090;W:310c;;\n");
    contains(out,"DECODED_NUL_TERMINATED=YES\n");
    contains(out,"DECODED_FIELD_ID=7654321\n");
    contains(out,"DECODED_FIELD_A=0129\n");
    contains(out,"DECODED_FIELD_M=0090\n");
    contains(out,"DECODED_FIELD_W=310c\n");
    contains(out,"DECODED_FIELD_SEMANTICS=UNVERIFIED\n");
    free(out);

    const uint8_t nontext[]={'a','b',0,'x'};
    out=format('V',nontext,sizeof nontext);
    contains(out,"REPLY_HEX=0x61 0x62 0x00 0x78\n");
    contains(out,"DECODED=UNRECOGNIZED_VERSION_REPLY\n");
    free(out);

    const uint8_t lines[]={0x00,0x00,0x01,0x2c};
    out=format('P',lines,sizeof lines);
    contains(out,"REPLY_HEX=0x00 0x00 0x01 0x2C\n");
    contains(out,"DECODED_PRINTED_LINES_CANDIDATE=300\n");
    contains(out,"DECODED_PRINTED_LINES_VERIFIED=NO\n");
    free(out);

    assert(leitz_probe_format(stdout,'A',NULL,0)==-1);
    assert(leitz_probe_format(stdout,'X',ready,1)==-1);
    puts("PASS direct probe formatting: raw hex, status codes, ASCII fields, NUL, printed-line candidate, unknown replies");
    return 0;
}
