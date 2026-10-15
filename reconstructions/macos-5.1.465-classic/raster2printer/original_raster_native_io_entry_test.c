/* Only verifies OEM named native C function signatures and field
 * boundaries. Does not execute socket or CUPS operations. */
#include "original_raster_native_io_entry.h"
#include <stddef.h>
_Static_assert(sizeof(classic_oem_read_descriptor)==0x10a8,
               "Original full raster descriptor extent");
_Static_assert(offsetof(classic_original_raster_descriptor,scheme)==0x50,
               "Native resolver scheme field");
_Static_assert(offsetof(classic_original_raster_descriptor,hostname)==0x450,
               "Native resolver hostname field");
_Static_assert(offsetof(classic_original_raster_descriptor,resource)==0x850,
               "Native resolver resource field");
_Static_assert(offsetof(classic_original_raster_descriptor,username)==0xc50,
               "Native resolver username field");
_Static_assert(offsetof(classic_original_raster_descriptor,port)==0x1098,
               "Native resolver port field");
_Static_assert(_Generic(&ReadPage,
  int (*)(classic_oem_read_descriptor *,cups_page_header2_t *,
          unsigned char **,int *,int *):1,default:0),
  "Original ReadPage register-argument ABI");
_Static_assert(_Generic(&ResolveDeviceURI,
  int (*)(classic_oem_read_descriptor *):1,default:0),
  "Original ResolveDeviceURI full-descriptor-pointer ABI");
int original_raster_native_io_abi_probe(void) { return 1; }
