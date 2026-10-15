/* Verify both original macOS stack-by-value OEM entry interfaces.
 * This probe never opens a CUPS stream or an actual device. */
#include "original_raster_native_document_entry.h"
_Static_assert(_Generic(&StartDocument,
               void (*)(classic_oem_read_descriptor):1,default:0),
               "OEM StartDocument has a single by-value descriptor");
_Static_assert(_Generic(&EndDocument,
               void (*)(classic_oem_read_descriptor):1,default:0),
               "OEM EndDocument has a single by-value descriptor");
int original_raster_native_document_abi_probe(void) {return 1;}
