#ifndef RASTER2PRINTER_BOUNDARIES_RECOVERED_H
#define RASTER2PRINTER_BOUNDARIES_RECOVERED_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
/* Recovered C for the original command-emission decisions only.
 * This is NOT a reconstructed raster2printer binary. */
size_t classic_raster_start_document(int high_resolution, uint8_t out[124]);
size_t classic_raster_start_page(uint16_t width, uint16_t height,
                                int high_resolution, uint8_t out[11]);
size_t classic_raster_end_page(int cut_every_label, uint8_t out[4]);
size_t classic_raster_end_document(int cut_every_label, int terminate_flag,
                                   uint8_t out[6]);
void classic_raster_flush_data_streams(FILE *stream, FILE *debug_dump);
#endif
