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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int emit(const mac_classic_job_io *io, const uint8_t *src, size_t len) {
  return len && io->write(io->write_ctx, src, len) != 0 ? -1 : 0;
}

/* Original _ProcessPage has geometry, margin, and page-specific branches that
 * are not yet fully recovered. This integration accepts only exact packed
 * row stride: no arbitrary CUPS row padding is silently treated as artwork. */
static int process(const mac_classic_job_io *io,
                   const mac_classic_job_config *cfg,
                   const mac_classic_page_info *p,
                   const uint8_t *pixels, unsigned page) {
  if (!p->width || p->width > 960 || !p->height || p->height > 65535u) return -1;
  const unsigned packed_stride = (p->width + 7u) / 8u;
  if (packed_stride > 120u) return -1;
  if (p->bits_per_pixel != 1 && p->bits_per_pixel != 24) return -1;
  if ((p->bits_per_pixel == 1 && p->bytes_per_line != packed_stride) ||
      (p->bits_per_pixel == 24 && p->bytes_per_line != p->width * 3u)) return -1;
  if (p->bits_per_pixel == 24 && cfg->quality_selector == 7) return -1;

  uint8_t *mono = NULL;
  if (p->bits_per_pixel == 24) {
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
  n = mac_classic_end_page(cfg->cut_each_label, cmd);
  if (!n || emit(io, cmd, n) != 0) goto finish;
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
    const int ok = process(io,cfg,&p,pixels,*pages_done+1u);
    free(pixels);
    if (ok != 0) return -1;
    ++*pages_done;
  }
  if (*pages_done == 0 || (io->cancelled && io->cancelled(io->cancel_ctx))) return -1;
  uint8_t end[6];
  len = mac_classic_end_document(cfg->cut_each_label,cfg->terminate_flag,end);
  return len && !emit(io,end,len) ? 0 : -1;
}
