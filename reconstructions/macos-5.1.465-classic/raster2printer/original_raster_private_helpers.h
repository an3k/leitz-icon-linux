#ifndef ORIGINAL_RASTER_PRIVATE_HELPERS_H
#define ORIGINAL_RASTER_PRIVATE_HELPERS_H
#include <stdint.h>
/* Original C++ source built into classic raster2printer; these exported
 * portable C names are reconstruction interfaces, not original C++ mangling. */
extern int terminate_flag;
void signal_handler(int signo);
const char *classic_strrchr_wrapper(const char *s, int c);
char *classic_strstr_wrapper(char *haystack,const char *needle);
uint16_t classic_os_swap_int16(uint16_t x);
#endif
