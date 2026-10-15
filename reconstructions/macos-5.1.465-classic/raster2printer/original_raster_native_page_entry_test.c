/* Compile-time native C ABI proof; uses the original verified
 * Mach-O register/stack call surfaces, without printing to hardware. */
#include "original_raster_native_page_entry.h"
_Static_assert(sizeof(classic_oem_read_descriptor)==0x10a8,
               "Original stack/by-value document image size");
_Static_assert(_Generic(&StartPage,
               void (*)(classic_oem_read_descriptor *):1,default:0),
               "OEM StartPage is one descriptor pointer");
_Static_assert(_Generic(&ProcessPage,
               int (*)(classic_oem_read_descriptor *,const uint8_t *,
                        unsigned,int):1,default:0),
               "OEM ProcessPage is four register arguments");
_Static_assert(_Generic(&EndPage,
               void (*)(classic_oem_read_descriptor):1,default:0),
               "OEM EndPage receives descriptor by value");
int classic_oem_page_native_signature_probe(void) { return 1; }
