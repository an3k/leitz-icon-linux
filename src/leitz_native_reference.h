#ifndef LEITZ_NATIVE_REFERENCE_H
#define LEITZ_NATIVE_REFERENCE_H
#include <stdint.h>
#include <stdio.h>

/* Independently authored reference framing based on manufacturer-published
 * Linux 1.0.3 command sequence. Not the production printer protocol. */
typedef struct {
  FILE *out;
  unsigned width, total_rows, rows_written, pending_blank, dpi_y;
  int failed, finished;
} leitz_native_reference;

int leitz_native_begin(leitz_native_reference *p, FILE *out,
                       unsigned width, unsigned rows, unsigned dpi_y,
                       unsigned length_units);
int leitz_native_row(leitz_native_reference *p, const uint8_t *packed,
                     unsigned row_bytes);
int leitz_native_end(leitz_native_reference *p);
int leitz_native_validate(FILE *in, unsigned *width, unsigned *row_count,
                          char *error, size_t error_size);
#endif
