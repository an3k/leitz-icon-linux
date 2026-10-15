#ifndef ORIGINAL_RASTER_PAGE_LIFECYCLE_H
#define ORIGINAL_RASTER_PAGE_LIFECYCLE_H
#include "original_raster_active_page_output.h"
#include <stdint.h>
#include <stddef.h>

/* OEM 5.1.465 _StartPage (0x100007380..0x100007555) and
 * _EndPage (0x1000089b0..0x100008ab1). Callback ABI is a
 * source-reconstruction bridge, not the original C++ ABI. */
typedef struct {
    void *context;
    int (*write_data)(void *, const uint8_t *, size_t,
                      const void *, size_t, int);
    void (*flush_data_streams)(void *);
    uint32_t *job_size;
    int socket_fd;
    /* Original StartPage 0x74e1, EndPage 0x8a59 and native
     * ProcessPage command emit sites read the global __ZL4sock
     * separately for every _WriteData call. NULL preserves the
     * isolated fixture's fixed socket_fd. */
    int (*current_socket_fd)(void *);
} classic_original_page_lifecycle_ops;

/* Whole OEM descriptor is 0x10a8 bytes; _StartPage takes a pointer,
 * _EndPage receives it by value at its original call site. */
void classic_original_start_page(const classic_original_page_lifecycle_ops *,
                                 const classic_oem_read_descriptor *);
void classic_original_end_page(const classic_original_page_lifecycle_ops *,
                               const classic_oem_read_descriptor *);

/* Adapters for existing classic_oem_page_output_io callbacks, with
 * callback context pointing to classic_original_page_lifecycle_ops. */
void classic_original_page_start_callback(void *,
                                          classic_oem_read_descriptor *);
void classic_original_page_end_callback(void *, const void *, size_t);
#endif
