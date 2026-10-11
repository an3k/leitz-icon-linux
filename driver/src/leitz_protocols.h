#ifndef LEITZ_PROTOCOL_H
#define LEITZ_PROTOCOL_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#define LEITZ_MAX_WIDTH 960u
#define LEITZ_MAX_ROWS 64000u

typedef enum {
  LEITZ_PROTOCOL_CURRENT = 0,
  LEITZ_PROTOCOL_NATIVE_LINUX_103 = 1
} leitz_protocol;

/* Exactly the same prepared page and pixel rows reach either encoder. */
typedef struct {
  unsigned width, total_rows, raster_rows, top, bottom, length_code, dpi_y;
} leitz_page;

typedef struct {
  const char *name;
  size_t state_size;
  int (*begin)(void *state, FILE *out, const leitz_page *page, int first_page);
  int (*row)(void *state, const uint8_t *pixels, unsigned width);
  int (*end_page)(void *state, int cut);
  int (*finish)(void *state);
  int (*validate)(FILE *data, unsigned *pages, unsigned *width,
                  char *reason, size_t reason_size);
} leitz_protocol_ops;

/* Unlinked modules are absent at runtime, without stub driver functions. */
const leitz_protocol_ops *leitz_protocol_get(leitz_protocol protocol);
#endif
