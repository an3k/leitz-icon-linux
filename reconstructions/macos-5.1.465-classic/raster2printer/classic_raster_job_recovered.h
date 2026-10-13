#ifndef CLASSIC_RASTER_JOB_RECOVERED_H
#define CLASSIC_RASTER_JOB_RECOVERED_H
#include <stddef.h>
#include <stdint.h>
#include "classic_read_page.h"
typedef struct {unsigned high_resolution,quality_selector,optimization,cut_every_page,terminate_flag;} classic_raster_job_options;
typedef struct {classic_page_source source;void *output_ctx;int (*write_data)(void *,const uint8_t *,size_t);void (*flush_data_streams)(void *);void (*host_progress)(void *,unsigned,unsigned,unsigned);int (*is_terminated)(void *);} classic_raster_job_io;
int classic_raster_job_run(const classic_raster_job_io *,const classic_raster_job_options *,unsigned *);
#endif
