/* Recovers the original flow between OEM macOS _raster2printer initial
 * descriptor memset and _StartDocument (Mach-O 0x1000045dc..0x10000548e).
 * Existing PPD preflight / six-choice interpreters are reused unchanged.
 * The callbacks represent CUPS APIs and by-value OEM data transfers;
 * this is not the original mangled x86-64 ABI nor a live print filter. */
#include "original_raster_option_initialization.h"
#include <string.h>

typedef struct {
    const classic_oem_option_initialization_io *io;
    void *ppd;
} marked_state;
static const char *choice(void *ctx,const char *name) {
    marked_state *s=ctx;
    return s->io->marked_choice(s->io->preflight.context,s->ppd,name);
}
int classic_original_raster_initialize_job(
    const classic_oem_option_initialization_io *io,
    int argc,char **argv,unsigned char descriptor[OEM_DESCRIPTOR_SIZE],
    classic_oem_preflight_state *state,int *early_return) {
    classic_original_raster_initialize_options(descriptor);
    /* The original cupRasterOpen result already resides at descriptor
     * offset 0 before _StartDocument receives the descriptor by value.
     * Keep that original pointer even if later PPD handling returns -1. */
    int ready=classic_original_raster_preflight(&io->preflight,argc,argv,
                                                 state,early_return);
    if (state->raster)
        memcpy(descriptor,&state->raster,sizeof(state->raster));
    if (!ready) return 0;
    marked_state m={io,state->ppd};
    classic_original_ppd_options_io picks={&m,choice};
    classic_original_raster_select_options(descriptor,&picks);
    io->free_options(io->preflight.context,state->option_count,state->options);
    io->close_ppd(io->preflight.context,state->ppd);
    classic_original_raster_report_options(descriptor,&picks);
    io->open_debug_file(io->preflight.context);
    io->select_stdout(io->preflight.context,
        classic_original_raster_option_u32(descriptor,
                                           OEM_PRINT_THROUGH_PRN_OFFSET)==0);
    unsigned char by_value[OEM_DESCRIPTOR_SIZE];
    memcpy(by_value,descriptor,sizeof by_value);
    io->start_document(io->preflight.context,by_value,sizeof by_value);
    return 1;
}
