/* From SHA-256-verified classic macOS raster2printer 5.1.465 x86-64:
 * __Z14signal_handleri @ 0x100004420
 * __ZL7strrchrUa9enable_ifIXLb1EEEPKci @ 0x100004570
 * __ZL6strstrUa9enable_ifIXLb1EEEPcPKc @ 0x100008cf0
 * __ZL12_OSSwapInt16t @ 0x100009fe0
 * __ZL16__libcpp_strrchrPKci @ 0x10000a010
 * __ZL15__libcpp_strstrPKcS0_ @ 0x10000a040
 * Original C++ wrapper chains are condensed only where the resulting
 * C function has identical visible operations and return values.
 */
#include "original_raster_private_helpers.h"
#include <stdio.h>
#include <string.h>
int terminate_flag;
void signal_handler(int signo) {
    (void)signo;
    fprintf(stderr,"\nINFO: Cancelling the job...");
    terminate_flag=1;
}
static const char *classic_libcpp_strrchr(const char *s,int c) {
    return strrchr(s,c);
}
const char *classic_strrchr_wrapper(const char *s,int c) {
    return classic_libcpp_strrchr(s,c);
}
static char *classic_libcpp_strstr(char *haystack,const char *needle) {
    return strstr(haystack,needle);
}
char *classic_strstr_wrapper(char *haystack,const char *needle) {
    return classic_libcpp_strstr(haystack,needle);
}
uint16_t classic_os_swap_int16(uint16_t x) {
    return (uint16_t)((((unsigned)x)<<8)|((unsigned)x>>8));
}
