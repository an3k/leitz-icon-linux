/* Native compile-only contract for recovered OEM _SendDebugFileToPrinter.
 * Ordinary printing, PRN file paths and real hardware are not exercised. */
#include "original_raster_native_prn_entry.h"
_Static_assert(sizeof(dbgprn_filename)==1024,
               "OEM original debug filename is 1024 bytes");
_Static_assert(sizeof(log_filename)==1024,
               "OEM original log filename is 1024 bytes");
_Static_assert(_Generic(&SendDebugFileToPrinter,
               int (*)(void):1,default:0),
               "OEM SendDebugFileToPrinter must return int");
static int (*original_prn_entry)(void)=SendDebugFileToPrinter;
int original_raster_native_prn_abi_probe(void) {
    return original_prn_entry();
}
