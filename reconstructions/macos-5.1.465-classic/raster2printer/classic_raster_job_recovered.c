/* Recovered OEM active order; this callback-level integration is newly
 * authored and does not reproduce the original Mach-O function ABI.
 * No OEM binary parity or original-device verification is claimed. */
#include "classic_raster_job_recovered.h"
#include "classic_process_page_modes_01.h"
#include "classic_process_page_modes_23.h"
#include "raster2printer_boundaries_recovered.h"
#include "raster2printer_dither_recovered.h"
#include <stdlib.h>
#include <stdint.h>

static int emit(const classic_raster_job_io *io,const uint8_t *p,size_t n) {
    return n ? io->write_data(io->output_ctx,p,n) : 0;
}
static int render(const classic_raster_job_io *io,
                  const classic_raster_job_options *opts,
                  const classic_page_result *page,
                  classic_mode23_job_cache *cache) {
    unsigned bpp=page->header.cups_bits_per_pixel;
    unsigned stride=page->header.cups_bytes_per_line/bpp+
                   (page->header.cups_bytes_per_line%bpp!=0);
    unsigned rows=page->header.cups_height;
    int rgb=page->header.cups_color_space==1u;
    const uint8_t *bitmap=page->pixels;
    uint8_t *converted=NULL,*line=NULL,header[11],footer[4];
    classic_mode01_state m01;
    classic_mode23_page m23;
    int rc=-1;
    if(rgb) {
        converted=calloc(rows,stride);
        if(!converted) goto finish;
        classic_raster_dither_rgb24_recovered(opts->quality_selector,
                     bitmap,rows,page->header.cups_bytes_per_line,converted);
        bitmap=converted;
    }
    line=malloc((size_t)stride+12u);
    if(!line) goto finish;
    size_t n=classic_raster_start_page((uint16_t)page->header.cups_width,
                     (uint16_t)rows,opts->high_resolution,header);
    if(emit(io,header,n)) goto finish;
    if(opts->optimization<=1) {
        uint8_t pre[4];
        classic_mode01_begin(&m01,opts->optimization,stride,pre);
        if(emit(io,pre,sizeof pre)) goto finish;
    } else classic_mode23_begin(&m23,cache,opts->optimization,stride,0);
    for(unsigned y=0;y<rows;++y) {
        if((y&31u)==0u && io->host_progress)
            io->host_progress(io->output_ctx,page->page_number,y,
                               (unsigned)((100ull*y)/rows));
        n=opts->optimization<=1 ?
          classic_mode01_row(&m01,bitmap+(size_t)y*stride,line) :
          classic_mode23_row(&m23,bitmap+(size_t)y*stride,line);
        if(emit(io,line,n)) goto finish;
        /* OEM _ProcessPage checks the termination flag after each row. */
        if(io->is_terminated && io->is_terminated(io->output_ctx)) goto finish;
    }
    n=classic_raster_end_page(opts->cut_every_page,footer);
    if(emit(io,footer,n)) goto finish;
    if(io->flush_data_streams) io->flush_data_streams(io->output_ctx);
    rc=0;
finish:
    free(converted);
    free(line);
    return rc;
}
int classic_raster_job_run(const classic_raster_job_io *io,
                           const classic_raster_job_options *opts,
                           unsigned *printed_pages) {
    classic_mode23_job_cache cache={0};
    unsigned page_index=0;
    uint8_t header[124],footer[6];
    if(!io || !opts || !printed_pages || !io->source.read_header ||
       !io->source.read_pixels || !io->write_data) return -1;
    *printed_pages=0;
    size_t n=classic_raster_start_document(opts->high_resolution,header);
    if(emit(io,header,n)) return -1;
    for(;;) {
        classic_page_result page={0};
        int read=classic_read_page_core(&io->source,&page_index,&page);
        if(read==0) break;
        if(read<0) {free(page.pixels);return -1;}
        /* This adapter cannot reconstruct undefined OEM behavior for an
         * RGB page with a Draft/1-bit table, or an absent bits-per-pixel. */
        if(!page.header.cups_bits_per_pixel ||
           !page.header.cups_bytes_per_line ||
           !page.header.cups_height ||
           (page.header.cups_color_space==1u &&
            (opts->quality_selector==7 ||
             page.header.cups_bits_per_pixel!=24))) {
            free(page.pixels);return -1;
        }
        int rc=render(io,opts,&page,&cache);
        free(page.pixels);
        if(rc) return -1;
        ++*printed_pages;
    }
    n=classic_raster_end_document(opts->cut_every_page,
                                   opts->terminate_flag,footer);
    return emit(io,footer,n) ? -1 : 0;
}
