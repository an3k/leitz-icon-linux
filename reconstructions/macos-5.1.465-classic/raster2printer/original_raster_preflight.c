/* Leitz Icon 5.1.465 macOS classic x86_64 raster2printer original routine
 * _raster2printer, 0x100004608..0x100004c74 (early CUPS and PPD setup).
 * Translated from the verified Mach-O original; stderr literals and source
 * line numbers 76,83,86,115,123,128 came directly from the binary.
 * The Linux 1.0.3 manufacturer C was used only for independent context:
 * its ContinuousPaperLength special-case is ABSENT from this Mac binary.
 */
#include "original_raster_preflight.h"
#include <stdio.h>
#include <stdlib.h>

int classic_original_raster_preflight(const classic_oem_preflight_io *io,
                                      int argc, char **argv,
                                      classic_oem_preflight_state *state,
                                      int *early_return) {
    setbuf(stderr, NULL);
    if (argc < 6 || argc > 7) {
        fprintf(stderr,
          "\nraster2printer.c:76:raster2printer(): error: (argc < 6) || (argc > 7)");
        *early_return=1;
        return 0;
    }
    state->input_fd=0;
    if (argc == 7) {
        fprintf(stderr,
          "\nraster2printer.c:83:raster2printer(): opening file from argv[6]='%s'",
          argv[6]);
        state->input_fd=io->open_input(io->context, argv[6], 0);
        if (state->input_fd==-1) {
            fprintf(stderr,
              "\nraster2printer.c:86:raster2printer(): cannot open file argv[6]='%s'",
              argv[6]);
            io->sleep_seconds(io->context,1);
            *early_return=1;
            return 0;
        }
    }
    state->raster=io->open_raster(io->context,state->input_fd,0);
    if (!state->raster) {
        perror("ERROR: Unable to open raster file - ");
        *early_return=1;
        return 0;
    }
    state->ppd=io->open_ppd(io->context,getenv("PPD"));
    if (!state->ppd) {
        perror("WARNING: Unable to open ppd file, use default settings - ");
        *early_return=-1;
        return 0;
    }
    state->options=NULL;
    state->option_count=io->parse_options(io->context,argv[5],0,&state->options);
    io->mark_defaults(io->context,state->ppd);
    for (int i=0;i<state->option_count;++i) {
        fprintf(stderr,
          "\nraster2printer.c:115:raster2printer(): option[%d]:'%s'='%s'",
          i,state->options[i].name,state->options[i].value);
        if (state->options[i].value) {
            int conflicts=io->mark_option(io->context,state->ppd,
                                        state->options[i].name,
                                        state->options[i].value);
            if (conflicts) {
                fprintf(stderr,
                  "\nraster2printer.c:123:raster2printer(): got %d conflicts for option '%s'",
                  conflicts,state->options[i].name);
            }
        }
    }
    fprintf(stderr,"\nraster2printer.c:128:raster2printer(): process options");
    io->mark_all_options(io->context,state->ppd,
                         state->option_count,state->options);
    return 1;
}
