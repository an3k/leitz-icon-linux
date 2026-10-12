#ifndef LEITZ_PROTOCOLS_H
#define LEITZ_PROTOCOLS_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#define LEITZ_MAX_WIDTH 960u
#define LEITZ_MAX_ROWS 64000u
#define LEITZ_PROTOCOL_ABI 1u

typedef enum {
  LEITZ_PROTOCOL_CURRENT = 0,
  LEITZ_PROTOCOL_NATIVE = 1,
  LEITZ_PROTOCOL_AUTO = 2
} leitz_protocol;

/* Shared logical page contract; encoder owns its device byte format. */
typedef struct {
  unsigned width, total_rows, raster_rows, top, bottom, length_code, dpi_y;
  /* Current only: 0 optimized, 1 full-width raw, 2 trimmed raw, 3 no-skip raw. */
  unsigned current_raster_mode;
} leitz_page;

typedef struct {
  unsigned abi_version;
  size_t struct_size;
  const char *name;
  size_t state_size;
  int (*begin)(void *state, FILE *out, const leitz_page *page, int first_page);
  int (*row)(void *state, const uint8_t *pixels, unsigned width);
  int (*end_page)(void *state, int cut);
  int (*finish)(void *state);
  int (*validate)(FILE *data, unsigned *pages, unsigned *width,
                  char *reason, size_t reason_size);
} leitz_protocol_ops;

/* Dynamically loaded on demand. Auto prefers Current, then Native.
 * Libraries remain loaded for this short-lived filter/CLI process only.
 * Each module exports the symbol "leitz_protocol_driver". */
const leitz_protocol_ops *leitz_protocol_get(leitz_protocol protocol);
#endif
