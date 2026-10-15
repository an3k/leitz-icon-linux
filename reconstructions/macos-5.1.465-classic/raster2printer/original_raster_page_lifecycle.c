/* Manufacturer macOS classic Leitz Icon 5.1.465 raster2printer.
 * Full defined StartPage and EndPage command/counter/flush sequence,
 * translated from SHA-256-verified Mach-O at 0x100007380..0x100007555
 * and 0x1000089b0..0x100008ab1.
 * Existing OEM command encoders are reused without duplicating bytes.
 * WriteData result is ignored, as in the manufacturer binary.
 */
#include "original_raster_page_lifecycle.h"
#include "raster2printer_boundaries_recovered.h"
#include <string.h>

static uint32_t field32(const classic_oem_read_descriptor *d,size_t offset) {
    uint32_t n;
    memcpy(&n,(const uint8_t*)d+offset,sizeof n);
    return n;
}
static void emit(const classic_original_page_lifecycle_ops *ops,
                 const uint8_t *bytes,size_t length,
                 const classic_oem_read_descriptor *descriptor) {
    unsigned char by_value[sizeof(*descriptor)];
    memcpy(by_value,descriptor,sizeof by_value);
    *ops->job_size+=(uint32_t)length;
    (void)ops->write_data(ops->context,bytes,length,
                          by_value,sizeof by_value,
                          ops->current_socket_fd ?
                          ops->current_socket_fd(ops->context) :
                          ops->socket_fd);
}
void classic_original_start_page(const classic_original_page_lifecycle_ops *ops,
                                 const classic_oem_read_descriptor *d) {
    uint8_t bytes[11];
    size_t n=classic_raster_start_page(d->cups_width,d->cups_height,
                                       field32(d,0x14)!=0,bytes);
    emit(ops,bytes,n,d);
}
void classic_original_end_page(const classic_original_page_lifecycle_ops *ops,
                               const classic_oem_read_descriptor *d) {
    uint8_t bytes[4];
    size_t n=classic_raster_end_page(field32(d,0x10)!=0,bytes);
    emit(ops,bytes,n,d);
    ops->flush_data_streams(ops->context); /* unconditional OEM call */
}
void classic_original_page_start_callback(void *context,
                                          classic_oem_read_descriptor *d) {
    classic_original_start_page(context,d);
}
void classic_original_page_end_callback(void *context,
                                        const void *by_value,size_t bytes) {
    (void)bytes; /* OEM caller passes precisely 0x10a8 bytes. */
    classic_original_end_page(context,by_value);
}
