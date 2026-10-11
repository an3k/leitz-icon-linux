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
/* Original Linux: reset/resolution once per document; one page header per
 * label. cut=0 emits ESC G between labels; the final page always emits ESC E. */
int leitz_native_begin_page(leitz_native_reference *p, FILE *out,
                            unsigned width, unsigned rows, unsigned dpi_y,
                            unsigned length_units, int first_page);
int leitz_native_finish_page(leitz_native_reference *p, int cut);
int leitz_native_finish_job(leitz_native_reference *p);
int leitz_native_validate_job(FILE *in, unsigned *pages_out,
                              unsigned *max_width, char *error,
                              size_t error_size);

int leitz_native_validate(FILE *in, unsigned *width, unsigned *row_count,
                          char *error, size_t error_size);
#endif
