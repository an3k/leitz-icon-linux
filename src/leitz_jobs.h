#ifndef LEITZ_JOBS_H
#define LEITZ_JOBS_H
#include "leitz_protocols.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* Protocol-neutral CUPS job coordinator. Media, page count and cutter policy
 * are shared. Only the wire encoder differs (Current vs NativeLinux103). */
typedef struct leitz_job leitz_job;
leitz_job *leitz_job_create(FILE *out, leitz_protocol protocol, int cut_each_page);
/* Optional source-derived plain/raw rows in Current; Native is unchanged. */
int leitz_job_set_current_full_width_raw(leitz_job *job, int enabled);
int leitz_job_begin_page(leitz_job *job, unsigned width, unsigned total_rows,
                         unsigned raster_rows, unsigned top, unsigned bottom,
                         unsigned length_code, unsigned dpi_y);
int leitz_job_row(leitz_job *job, const uint8_t *pixels);
int leitz_job_end_page(leitz_job *job);
int leitz_job_finish(leitz_job *job);
int leitz_job_validate(leitz_job *job, char *reason, size_t reason_size);
unsigned leitz_job_page_count(const leitz_job *job);
const char *leitz_job_protocol_name(const leitz_job *job);
void leitz_job_destroy(leitz_job *job);
#endif
