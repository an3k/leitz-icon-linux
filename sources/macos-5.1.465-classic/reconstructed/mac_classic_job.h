/* Classic macOS 5.1.465 CUPS job path: portable reconstruction of dispatch.
 * Binary evidence: raster2printer _raster2printer 0x1000045a0..0x100006854,
 * calls to _StartDocument at 0x10000548e, _DitherPage 0x100005fb9,
 * _StartPage 0x100005fc5, _ProcessPage 0x100006011 / 0x100006189,
 * _EndPage 0x10000605e / 0x1000061d6, _EndDocument 0x10000632d.
 * Not the manufacturer's original C or a complete CUPS filter.
 */
#ifndef MAC_CLASSIC_JOB_H
#define MAC_CLASSIC_JOB_H
#include "mac_classic_read_page.h"
#include <stddef.h>
#include <stdint.h>

typedef int (*mac_classic_write_fn)(void *ctx, const uint8_t *bytes, size_t len);
typedef struct {
  unsigned optimization_level; /* PPD OptimizationLevel: 0..3 */
  unsigned quality_selector;   /* PPD PrintQuality selector: 0..14 */
  unsigned vertical_dpi;       /* 300 or 600 */
  int cut_each_label;          /* PPD LabelCutter ON */
  int terminate_flag;          /* cancellation after requested pages */
} mac_classic_job_config;

/* Progress describes rows submitted to the encoder, not printer feedback. */
typedef void (*mac_classic_progress_fn)(void *ctx, unsigned page,
                                        unsigned row, unsigned percent);
/* Mirrors the original global SIGTERM cancellation check as a callback. */
typedef int (*mac_classic_cancel_fn)(void *ctx);
/* _EndPage calls _FlushDataStreams immediately after ESC G[/ESC E].
 * An explicit callback preserves this original per-page flush boundary.
 * A failed flush is fatal, rather than ignored as in the OEM routine. */
typedef int (*mac_classic_flush_fn)(void *write_ctx);
typedef struct {
  const mac_classic_page_source *source;
  void *read_ctx;
  mac_classic_write_fn write;
  void *write_ctx;
  mac_classic_progress_fn progress; /* optional, CUPS ATTR on every 32nd row */
  void *progress_ctx;
  mac_classic_cancel_fn cancelled; /* optional: stop before transmitting another row */
  void *cancel_ctx;
  mac_classic_flush_fn flush; /* optional: called once after each completed page */
} mac_classic_job_io;

/* No USB, no retries. Returns -1 on bad input, short read, write failure or
 * cancellation; never falsely claims the printer has mechanically finished.
 * The stream can be incomplete after a write failure: do NOT automatically
 * transmit it to a physical printer. */
int mac_classic_job_run(const mac_classic_job_io *io,
                        const mac_classic_job_config *config,
                        unsigned *pages_done);
#endif
