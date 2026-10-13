/* CUPS-independent classic macOS job coordinator, recovered call ordering.
 * This is a *new safe integration layer* connecting individually recovered
 * manufacturer functions; the exact original decision logic is NOT recovered.
 * References: 0x10000548e, 0x100005fb9, 0x100005fc5, 0x100006011,
 * 0x10000605e, 0x10000632d of classic raster2printer Mach-O.
 */
#include "mac_classic_job.h"
#include "mac_classic_dither.h"
#include "mac_classic_raster_boundaries.h"
#include "mac_classic_process_page.h"
#include "mac_classic_progress.h"
#include "mac_classic_geometry.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int emit(const mac_classic_job_io *io, const uint8_t *src, size_t len) {
  return len && io->write(io->write_ctx, src, len) != 0 ? -1 : 0;
}

/* Original _ProcessPage has geometry, margin, and page-specific branches
 * not yet fully recovered. The packed stride below comes from _ReadPage's
 * CUPS byte count / bits-per-pixel, not the CUPS page width. */
static int process(const mac_classic_job_io *io,
                   const mac_classic_job_config *cfg,
                   const mac_classic_page_info *p,
                   const uint8_t *pixels, unsigned page,
                   unsigned *previous_optimized_width) {
  unsigned packed_stride = 0;
  if (mac_classic_packed_stride(p, &packed_stride) != 0) return -1;
  /* Original _raster2printer chooses _DitherPage by cupsColorSpace == 1,
   * copied from cups_page_header2_t +0x190 at 0x100005831..0x100005837
   * and compared at 0x100005f84. BitsPerPixel alone is NOT the switch.
   * Unsupported combinations must not be silently treated as RGB/K. */
  const int rgb = p->color_space == 1u; /* CUPS_CSPACE_RGB */
  if ((rgb && p->bits_per_pixel != 24) ||
      (!rgb && p->bits_per_pixel != 1) ||
      (rgb && cfg->quality_selector == 7)) return -1;

  uint8_t *mono = NULL;
  if (rgb) {
    if (p->height > SIZE_MAX / packed_stride) return -1;
    mono = calloc(p->height, packed_stride);
    if (!mono) return -1;
    if (mac_classic_dither_rgb24(cfg->quality_selector, pixels,
          (size_t)p->height * p->bytes_per_line, p->height,
          p->bytes_per_line, mono, (size_t)p->height * packed_stride) != 0) {
      free(mono); return -1;
    }
  }
  const uint8_t *bitmap = mono ? mono : pixels;
  uint8_t cmd[124];
  int ret = -1;
  if (mac_classic_start_page((uint16_t)p->width, (uint16_t)p->height,
                            cfg->vertical_dpi == 600, cmd) != 11 ||
      emit(io, cmd, 11) != 0) goto finish;

  mac_classic_raster_row state;
  if (mac_classic_row_init(&state, cfg->optimization_level, packed_stride) != 0)
    goto finish;
  /* OEM mode 2's static bytesPrintedInPreviousLine and mode 3's
   * descriptor+0x24 retain their value across page calls. */
  if (cfg->optimization_level >= 2) state.previous_width = *previous_optimized_width;
  uint8_t header[4];
  size_t n = mac_classic_row_preamble(&state, header);
  if (emit(io, header, n) != 0) goto finish;

  for (unsigned y=0; y<p->height; ++y) {
    /* Classic _ProcessPage inspects _terminate_flag at the row boundary;
     * on cancellation stop without claiming a completed/cut page. */
    if (io->cancelled && io->cancelled(io->cancel_ctx)) goto finish;
    unsigned percent = 0;
    int report = mac_classic_raster_progress(y,p->height,&percent);
    if (report < 0) goto finish;
    if (report && io->progress) io->progress(io->progress_ctx,page,y,percent);
    uint8_t encoded[1024];
    size_t length = 0;
    const uint8_t *row = bitmap + (size_t)y * packed_stride;
    if (mac_classic_row_encode(&state, row, encoded, sizeof encoded, &length))
      goto finish;
    if (emit(io, encoded, length) != 0) goto finish;
  }
  /* Original _ProcessPage checks terminate_flag after submitting every
   * row (0x100008945), including the last one. If SIGTERM arrives during
   * the final write, a pre-row-only check incorrectly sends ESC G/E. */
  if (io->cancelled && io->cancelled(io->cancel_ctx)) goto finish;
  n = mac_classic_end_page(cfg->cut_each_label, cmd);
  if (!n || emit(io, cmd, n) != 0) goto finish;
  /* OEM _EndPage invokes _FlushDataStreams at 0x100008a8d after the
   * page-end command, before _ReadPage proceeds to another label.
   * Preserve that boundary for stdout and for deferred PRN output. */
  if (io->flush && io->flush(io->write_ctx) != 0) goto finish;
  if (cfg->optimization_level >= 2) *previous_optimized_width=state.previous_width;
  ret = 0;
finish:
  free(mono);
  return ret;
}

int mac_classic_job_run(const mac_classic_job_io *io,
                        const mac_classic_job_config *cfg, unsigned *pages_done) {
  if (!io || !io->source || !io->source->read_header ||
      !io->source->read_pixels || !io->write || !cfg || !pages_done ||
      (cfg->vertical_dpi != 300 && cfg->vertical_dpi != 600) ||
      cfg->optimization_level > 3 || cfg->quality_selector >= 15) return -1;
  *pages_done = 0;
  if (io->cancelled && io->cancelled(io->cancel_ctx)) return -1;
  uint8_t start[124];
  size_t len = mac_classic_start_document(cfg->vertical_dpi == 600, start);
  if (!len || emit(io,start,len)) return -1;

  unsigned read_pages = 0;
  unsigned previous_optimized_width = 0;
  for (;;) {
    if (io->cancelled && io->cancelled(io->cancel_ctx)) return -1;
    mac_classic_page_info p = {0};
    uint8_t *pixels = NULL;
    size_t bytes = 0;
    int result = mac_classic_read_page(io->source, io->read_ctx,
                                      &p, &pixels, &bytes, &read_pages);
    if (result == MAC_CLASSIC_PAGE_EOF) break;
    if (result != MAC_CLASSIC_PAGE_OK) return -1;
    if (!p.width || !p.bits_per_pixel || !p.bytes_per_line || !p.height ||
        bytes != (size_t)p.bytes_per_line * p.height) {
      free(pixels); return -1;
    }
    const int ok = process(io,cfg,&p,pixels,*pages_done+1u,
                           &previous_optimized_width);
    free(pixels);
    if (ok != 0) return -1;
    ++*pages_done;
  }
  if (*pages_done == 0 || (io->cancelled && io->cancelled(io->cancel_ctx))) return -1;
  uint8_t end[6];
  len = mac_classic_end_document(cfg->cut_each_label,cfg->terminate_flag,end);
  return len && !emit(io,end,len) ? 0 : -1;
}
