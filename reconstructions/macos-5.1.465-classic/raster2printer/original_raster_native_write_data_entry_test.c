/* Compile-time original raster _WriteData ABI proof. Real output
 * and PRN behavior are verified by the existing authenticated
 * manufacturer-versus-CUPS full-program comparisons. */
#include "original_raster_native_write_data_entry.h"
#include "original_device_uri.h"
#include <stddef.h>
_Static_assert(sizeof(classic_oem_read_descriptor)==0x10a8u,
               "OEM 0x215 x 8 descriptor size");
_Static_assert(offsetof(classic_original_raster_descriptor,scheme)==0x50u,
               "OEM scheme offset");
typedef int (*oem_native_write_data)(
    const uint8_t *,size_t,int,classic_oem_read_descriptor);
static oem_native_write_data native_entry=WriteData;
int classic_raster_native_write_data_abi_probe(
        const uint8_t *bytes,size_t count,int socket_fd,
        classic_oem_read_descriptor descriptor) {
    return native_entry(bytes,count,socket_fd,descriptor);
}
