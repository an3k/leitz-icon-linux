#ifndef ORIGINAL_RASTER_PRIVATE_HELPERS_H
#define ORIGINAL_RASTER_PRIVATE_HELPERS_H
#include <stdint.h>
/* Original C++ source in classic raster2printer. Most functions retain
 * portable C analysis names because their original C++ ABI differs.
 * The signal handler, however, takes only an int and has the matching
 * system calling convention. On macOS use the exact OEM C++ Mach-O symbol
 * __Z14signal_handleri, checked against authenticated original nm output.
 * This is a symbol-name bridge, not a claim of full C++ global-state ABI. */
extern int terminate_flag;
#if defined(__APPLE__)
void signal_handler(int signo) __asm__("__Z14signal_handleri");
#else
void signal_handler(int signo);
#endif
/* Source-level C signatures reproduce the OEM wrapper calls, while
 * Apple assembler labels recover the original file-private C++ symbol
 * spellings seen at 0x100004570, 0x100008cf0 and 0x100009fe0.
 * As with the signal handler, exact original C++ object linkage and
 * compiler inlining are not implied by these function-name aliases. */
#if defined(__APPLE__)
const char *classic_strrchr_wrapper(const char *s,int c)
    __asm__("__ZL7strrchrUa9enable_ifIXLb1EEEPKci");
char *classic_strstr_wrapper(char *haystack,const char *needle)
    __asm__("__ZL6strstrUa9enable_ifIXLb1EEEPcPKc");
uint16_t classic_os_swap_int16(uint16_t x)
    __asm__("__ZL12_OSSwapInt16t");
#else
const char *classic_strrchr_wrapper(const char *s,int c);
char *classic_strstr_wrapper(char *haystack,const char *needle);
uint16_t classic_os_swap_int16(uint16_t x);
#endif
#endif
