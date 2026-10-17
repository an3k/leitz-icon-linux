#ifndef LEITZ_WIN2018_GPD_JOB_SEQUENCE_H
#define LEITZ_WIN2018_GPD_JOB_SEQUENCE_H
#include <stddef.h>
#include <stdint.h>

/* Portable static GPD command expansion, not a Windows Unidrv/OEM ABI.
 * These values represent the literal original GPD option keys. */
typedef enum {
    LEITZ_WIN2018_RES_OPTION1 = 1, /* GPD: 300x300 dpi -> ESC h; default */
    LEITZ_WIN2018_RES_OPTION2 = 2  /* GPD: 300x600 dpi -> ESC i */
} leitz_win2018_gpd_resolution;

typedef int32_t (*leitz_win2018_gpd_job_write)(void *context,
                                               const uint8_t *data,
                                               size_t size);

/* On a valid selection, emit in the static order written in the GPD:
 * DOC_SETUP.1: CmdStartDoc;
 * PAGE_SETUP.1: CmdStartPage; PAGE_SETUP.2: PaperSize CmdSelect;
 * PAGE_SETUP.3: Resolution CmdSelect;
 * PAGE_FINISH.1: CmdEndPage (empty); JOB_FINISH.1: CmdEndJob.
 * A nonzero callback return aborts this portable adapter. OEM host
 * callback/error/packet semantics have NOT been recovered. */
int leitz_win2018_gpd_begin_document(leitz_win2018_gpd_job_write write,
                                     void *context);
int leitz_win2018_gpd_setup_page(const char *original_paper_id,
                                 leitz_win2018_gpd_resolution resolution,
                                 leitz_win2018_gpd_job_write write,
                                 void *context);
int leitz_win2018_gpd_end_page(void); /* GPD CmdEndPage is empty */
int leitz_win2018_gpd_end_job(leitz_win2018_gpd_job_write write,
                              void *context);
#endif
