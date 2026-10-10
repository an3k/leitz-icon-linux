#ifndef LEITZ_CODEC_H
#define LEITZ_CODEC_H
#include <stdio.h>
#include <stdint.h>
#define LEITZ_MAX_WIDTH 960
#define LEITZ_MAX_ROWS 64000

typedef struct {
 FILE *out;
 unsigned width, raster_rows, total_rows, emitted_rows, printed_rows;
 unsigned pending_blank, feed_units, top, bottom;
 int previous_width, error;
} leitz_encoder;
/* One-dimensional printer command language, separate from CUPS and PDF. */
int leitz_begin_media(leitz_encoder *e,FILE *out,unsigned width,unsigned total_rows,
   unsigned raster_rows,unsigned top,unsigned bottom,unsigned length_code,unsigned dpi_y);
int leitz_row_media(leitz_encoder *e,const uint8_t *pixels);
int leitz_end_page(leitz_encoder *e, int cut);
int leitz_end(leitz_encoder *e);
int leitz_validate(FILE *f,unsigned *pages,char *reason,size_t reason_size);
#endif
