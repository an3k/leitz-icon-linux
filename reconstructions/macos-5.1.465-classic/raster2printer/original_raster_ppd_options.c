/* Extracted OEM macOS 5.1.465 raster2printer option interpretation:
 * original Mach-O 0x100004c74..0x100005412, verified binary SHA-256
 * 72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13.
 * The exact option selection, case sensitivity, defaults, diagnostic and
 * descriptor offsets are retained. The calling CUPS API sequence is NOT
 * fully reconstructed in this independent function interface.
 */
#include "original_raster_ppd_options.h"
#include "raster2printer_quality_recovered.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static void store_u32(unsigned char *descriptor,unsigned offset,uint32_t value) {
    memcpy(descriptor+offset,&value,sizeof(value));
}
uint32_t classic_original_raster_option_u32(const unsigned char *descriptor,
                                             unsigned offset) {
    uint32_t x;memcpy(&x,descriptor+offset,sizeof(x));return x;
}
void classic_original_raster_initialize_options(unsigned char *descriptor) {
    memset(descriptor,0,OEM_DESCRIPTOR_SIZE);
    store_u32(descriptor,OEM_OPTIMIZATION_OFFSET,1);
}
static const char *choice(const classic_original_ppd_options_io *io,
                          const char *key) {
    return io->marked_choice(io->context,key);
}
void classic_original_raster_select_options(
    unsigned char *descriptor,const classic_original_ppd_options_io *io) {
    const char *c=choice(io,"Resolution");
    if(c) store_u32(descriptor,OEM_HIGH_RESOLUTION_OFFSET,
                    strcasecmp(c,"600dpi")==0);
    c=choice(io,"PrintQuality");
    if(c) {
        /* Original line 142 diagnostic occurs before comparisons. */
        fprintf(stderr,
                "\nraster2printer.c:142:raster2printer(): PrintQuality choice->choice = %s",c);
        int n=classic_raster_quality_selector(c);
        if(n>=0) store_u32(descriptor,OEM_PRINT_QUALITY_OFFSET,(uint32_t)n);
    }
    c=choice(io,"DebugMode");
    if(c) {
        if(strcmp(c,"Mesh")==0) store_u32(descriptor,OEM_DEBUG_MODE_OFFSET,2);
        else if(strcmp(c,"Debug")==0) store_u32(descriptor,OEM_DEBUG_MODE_OFFSET,1);
        else if(strcmp(c,"PrintToFile")==0) store_u32(descriptor,OEM_DEBUG_MODE_OFFSET,3);
    }
    c=choice(io,"PrintThroughPRN");
    if(c) store_u32(descriptor,OEM_PRINT_THROUGH_PRN_OFFSET,
                    strcmp(c,"ON")==0);
    c=choice(io,"OptimizationLevel");
    if(c) {
        /* OEM atoi stores to 32 bits, then compares as UNSIGNED with 4. */
        uint32_t n=(uint32_t)atoi(c);
        store_u32(descriptor,OEM_OPTIMIZATION_OFFSET,n<4u?n:0u);
    }
    c=choice(io,"LabelCutter");
    if(c) store_u32(descriptor,OEM_CUT_EVERY_LABEL_OFFSET,
                    strcmp(c,"ON")==0);
}
void classic_original_raster_report_options(
    const unsigned char *descriptor,const classic_original_ppd_options_io *io) {
    (void)io;
    fprintf(stderr,
       "\nraster2printer.c:249:raster2printer(): options: resolution:%s; dbg-mod:%d; opt-lvl:%d; cut-every:%s",
       classic_original_raster_option_u32(descriptor,OEM_HIGH_RESOLUTION_OFFSET)?"hi":"low",
       (int)classic_original_raster_option_u32(descriptor,OEM_DEBUG_MODE_OFFSET),
       (int)classic_original_raster_option_u32(descriptor,OEM_OPTIMIZATION_OFFSET),
       classic_original_raster_option_u32(descriptor,OEM_CUT_EVERY_LABEL_OFFSET)?"yes":"no");
}
