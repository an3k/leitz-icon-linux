/* Manufacturer-binary-informed offline decoders. No printer I/O, no retry. */
#include "leitz_status.h"
#include <string.h>

int leitz_reply_kind_from_name(const char *name, leitz_reply_kind *out) {
  if (!name || !out) return -1;
  if (!strcmp(name, "legacy")) *out=LEITZ_REPLY_STATUS_LEGACY;
  else if (!strcmp(name, "v3")) *out=LEITZ_REPLY_STATUS_V3;
  else if (!strcmp(name, "lines")) *out=LEITZ_REPLY_PRINTED_LINES;
  else if (!strcmp(name, "version")) *out=LEITZ_REPLY_VERSION;
  else return -1;
  return 0;
}

int leitz_parse_reply(leitz_reply_kind kind, const uint8_t *raw, size_t size,
                      leitz_reply *out) {
  if (!out || !raw || !size || size>LEITZ_REPLY_MAX) return -1;
  memset(out, 0, sizeof *out);
  switch (kind) {
    case LEITZ_REPLY_STATUS_LEGACY:
      /* One status byte is known to be inspected; extra bytes are not decoded. */
      if (size>64) return -1;
      out->status_byte=raw[0];
      switch (raw[0]) {
        case 0x03: out->interpretation="READY_LEGACY"; break;
        case 0x20: out->interpretation="MEDIA_EMPTY_LEGACY"; break;
        case 0x30: out->interpretation="PAPER_JAM_LEGACY"; break;
        case 0x40: out->interpretation="GENERAL_ERROR_LEGACY"; break;
        default: out->interpretation="UNKNOWN_LEGACY"; break;
      }
      return 0;
    case LEITZ_REPLY_STATUS_V3:
      if (size>64) return -1;
      out->status_byte=raw[0];
      if (raw[0]==0x03) out->interpretation="READY_V3_UNVERIFIED";
      else if (raw[0]==0x04) out->interpretation="DOOR_OPEN_V3_UNVERIFIED";
      else {
        /* Earlier static notes give an active-low busy bit that overlaps
         * every listed 0xC0/0xA0/0x90/0x88 masked fault code. Until branch
         * precedence is re-verified, these MUST remain ambiguous. */
        const uint8_t masked=raw[0]&0xfc;
        if (masked==0xc0 || masked==0xa0 || masked==0x90 || masked==0x88) {
          out->interpretation="FAULT_OR_BUSY_V3_AMBIGUOUS";
          out->ambiguous=1;
        } else if ((raw[0]&0x02)==0) {
          out->interpretation="BUSY_V3_UNVERIFIED";
        } else out->interpretation="UNKNOWN_V3";
      }
      return 0;
    case LEITZ_REPLY_PRINTED_LINES:
      /* V3 sums big-endian bytes; exact reply size isn't established.
       * Treat 1..4-byte samples as candidate values, NOT completion proof. */
      if (size>4) return -1;
      for (size_t i=0;i<size;i++)
        out->printed_lines=(out->printed_lines<<8)|raw[i];
      out->interpretation="CANDIDATE_PRINTED_LINES_UNVERIFIED";
      return 0;
    case LEITZ_REPLY_VERSION: {
      /* Native macOS treats ESC V as text, but device NUL-termination is not
       * proven. Accept printable bytes with optional final NUL only. */
      const size_t n=(raw[size-1]==0)?size-1:size;
      if (!n) return -1;
      for (size_t i=0;i<n;i++) if (raw[i]<0x20 || raw[i]>0x7e) return -1;
      memcpy(out->version,raw,n);
      out->version[n]=0;
      out->interpretation="VERSION_TEXT_UNVERIFIED";
      return 0;
    }
  }
  return -1;
}
