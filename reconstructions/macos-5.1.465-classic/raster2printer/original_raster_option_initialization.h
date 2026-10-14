#ifndef CLASSIC_ORIGINAL_RASTER_OPTION_INITIALIZATION_H
#define CLASSIC_ORIGINAL_RASTER_OPTION_INITIALIZATION_H
#include "original_raster_preflight.h"
#include "original_raster_ppd_options.h"

typedef struct {
    classic_oem_preflight_io preflight;
    const char *(*marked_choice)(void *,void *,const char *);
    void (*free_options)(void *,int,classic_oem_cups_option *);
    void (*close_ppd)(void *,void *);
    void (*open_debug_file)(void *);
    void (*select_stdout)(void *,int);
    void (*start_document)(void *,const unsigned char *,unsigned);
} classic_oem_option_initialization_io;

/* Original _raster2printer early path through _StartDocument
 * 0x1000045dc..0x10000548e. 1=proceed, 0=OEM early return
 * (original result in *early_return). This is not the native ABI. */
int classic_original_raster_initialize_job(
    const classic_oem_option_initialization_io *io,
    int argc,char **argv,unsigned char descriptor[OEM_DESCRIPTOR_SIZE],
    classic_oem_preflight_state *state,int *early_return);
#endif
