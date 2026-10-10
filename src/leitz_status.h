#ifndef LEITZ_STATUS_H
#define LEITZ_STATUS_H
#include <stddef.h>
#include <stdint.h>

#define LEITZ_REPLY_MAX 1024u

typedef enum {
  LEITZ_REPLY_STATUS_LEGACY,
  LEITZ_REPLY_STATUS_V3,
  LEITZ_REPLY_PRINTED_LINES,
  LEITZ_REPLY_VERSION
} leitz_reply_kind;

typedef struct {
  uint8_t status_byte;
  uint32_t printed_lines;
  char version[LEITZ_REPLY_MAX + 1];
  const char *interpretation;
  int ambiguous;
} leitz_reply;

/* Parse captured response BYTES only; never talks to USB or authorizes printing. */
int leitz_parse_reply(leitz_reply_kind kind, const uint8_t *raw, size_t size,
                      leitz_reply *out);
int leitz_reply_kind_from_name(const char *name, leitz_reply_kind *out);
#endif
