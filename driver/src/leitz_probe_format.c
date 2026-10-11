/* Human-readable, conservative presentation of a captured Leitz reply.
 * The manufacturer status decoder is the shared source of interpretation.
 * Never change printer behavior based on this diagnostic output.
 */
#include "leitz_probe_format.h"
#include "leitz_status.h"
#include <ctype.h>
#include <string.h>

static void print_ascii_fields(FILE *out, const char *text) {
    const char *cursor=text;
    while (*cursor) {
        const char *end=strchr(cursor,';');
        if (!end) break;
        const char *colon=memchr(cursor,':',(size_t)(end-cursor));
        if (colon && colon>cursor && colon+1<end) {
            const size_t key_len=(size_t)(colon-cursor);
            int valid=key_len<=24 && isupper((unsigned char)cursor[0]);
            for (size_t i=1;valid && i<key_len;i++)
                if (!isupper((unsigned char)cursor[i]) &&
                    !isdigit((unsigned char)cursor[i]) &&
                    cursor[i]!='_') valid=0;
            for (const char *p=colon+1;valid && p<end;p++)
                if (!isalnum((unsigned char)*p) &&
                    *p!='.' && *p!='-' && *p!='_') valid=0;
            if (valid)
                fprintf(out,"DECODED_FIELD_%.*s=%.*s\n",
                        (int)key_len,cursor,(int)(end-colon-1),colon+1);
        }
        cursor=end+1;
    }
}

int leitz_probe_format(FILE *output, uint8_t opcode,
                       const uint8_t *bytes, size_t length) {
    if (!output || !bytes || length<1 || length>LEITZ_REPLY_MAX ||
        (opcode!='A' && opcode!='V' && opcode!='P')) return -1;
    fprintf(output,"COMMAND=ESC %c\nREPLY_BYTES=%zu\nREPLY_HEX=",opcode,length);
    for (size_t i=0;i<length;i++)
        fprintf(output,"%s0x%02X",i?" ":"",(unsigned)bytes[i]);
    fputc('\n',output);

    leitz_reply decoded;
    if (opcode=='A') {
        leitz_reply v3;
        if (!leitz_parse_reply(LEITZ_REPLY_STATUS_LEGACY,bytes,length,&decoded) &&
            !leitz_parse_reply(LEITZ_REPLY_STATUS_V3,bytes,length,&v3)) {
            fprintf(output,"DECODED_STATUS_HEX=0x%02X\n",(unsigned)decoded.status_byte);
            fprintf(output,"DECODED_STATUS_LEGACY=%s\n",decoded.interpretation);
            fprintf(output,"DECODED_STATUS_V3=%s\n",v3.interpretation);
            if (length>1) fputs("DECODED_EXTRA_STATUS_BYTES=UNINTERPRETED\n",output);
        } else fputs("DECODED=UNRECOGNIZED_STATUS_REPLY\n",output);
    } else if (opcode=='V') {
        if (!leitz_parse_reply(LEITZ_REPLY_VERSION,bytes,length,&decoded)) {
            fprintf(output,"DECODED_TEXT=%s\n",decoded.version);
            fprintf(output,"DECODED_NUL_TERMINATED=%s\n",
                    bytes[length-1]==0?"YES":"NO");
            print_ascii_fields(output,decoded.version);
            fputs("DECODED_FIELD_SEMANTICS=UNVERIFIED\n",output);
        } else fputs("DECODED=UNRECOGNIZED_VERSION_REPLY\n",output);
    } else {
        if (!leitz_parse_reply(LEITZ_REPLY_PRINTED_LINES,bytes,length,&decoded)) {
            fprintf(output,"DECODED_PRINTED_LINES_CANDIDATE=%u\n",decoded.printed_lines);
            fputs("DECODED_PRINTED_LINES_VERIFIED=NO\n",output);
        } else fputs("DECODED=UNRECOGNIZED_PRINTED_LINES_REPLY\n",output);
    }
    return fflush(output)==0 && !ferror(output) ? 0 : -1;
}
