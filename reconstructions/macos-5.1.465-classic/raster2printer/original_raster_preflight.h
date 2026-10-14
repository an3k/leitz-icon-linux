#ifndef CLASSIC_ORIGINAL_RASTER_PREFLIGHT_H
#define CLASSIC_ORIGINAL_RASTER_PREFLIGHT_H

/* Recovered early _raster2printer control flow: 0x100004608..0x100004c74.
 * The callback ABI is newly authored; this is not the manufacturer's ABI.
 * Source and state remain caller-owned, exactly as at this point of the OEM
 * routine. No cleanup is introduced on the original early-return paths. */
typedef struct {
    char *name;
    char *value;
} classic_oem_cups_option;

typedef struct {
    int input_fd;
    void *raster;
    void *ppd;
    int option_count;
    classic_oem_cups_option *options;
} classic_oem_preflight_state;

typedef struct {
    void *context;
    int (*open_input)(void *, const char *, int);
    void (*sleep_seconds)(void *, unsigned);
    void *(*open_raster)(void *, int, int);
    void *(*open_ppd)(void *, const char *);
    int (*parse_options)(void *, const char *, int, classic_oem_cups_option **);
    void (*mark_defaults)(void *, void *);
    int (*mark_option)(void *, void *, const char *, const char *);
    int (*mark_all_options)(void *, void *, int, classic_oem_cups_option *);
} classic_oem_preflight_io;

/* Returns 1 when the original continues into ppdFindMarkedChoice;
 * returns 0 for an original early return, writing original return value
 * (1 or -1) to *early_return. This is an analysis-only return contract. */
int classic_original_raster_preflight(const classic_oem_preflight_io *io,
                                      int argc, char **argv,
                                      classic_oem_preflight_state *state,
                                      int *early_return);
#endif
