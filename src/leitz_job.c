/* Common document/page/cutter coordinator for BOTH wire formats.
 * This module is the only place that selects a manufacturer/Current codec.
 * The CUPS raster filter handles media/geometry only; the USB backend handles
 * preflight/transfer only. No CUPS API, file paths or USB logic here.
 */
#include "leitz_job.h"
#include "leitz_codec.h"
#include "leitz_native_reference.h"
#include <stdlib.h>
#include <string.h>

struct leitz_job {
    FILE *out;
    leitz_protocol protocol;
    int cut_each_page;
    unsigned pages, first_width, first_dpi, width, remaining, bottom;
    int page_open, completed, failed;
    union {
        leitz_encoder current;
        leitz_native_reference native;
    } wire;
};

leitz_job *leitz_job_create(FILE *out, leitz_protocol protocol, int cut_each_page) {
    if (!out || (protocol != LEITZ_PROTOCOL_CURRENT &&
                 protocol != LEITZ_PROTOCOL_NATIVE_LINUX_103) ||
        (cut_each_page != 0 && cut_each_page != 1)) return NULL;
    leitz_job *job=calloc(1,sizeof *job);
    if (!job) return NULL;
    job->out=out;
    job->protocol=protocol;
    job->cut_each_page=cut_each_page;
    return job;
}

int leitz_job_begin_page(leitz_job *job, unsigned width, unsigned total_rows,
                         unsigned raster_rows, unsigned top, unsigned bottom,
                         unsigned length_code, unsigned dpi_y) {
    if (!job || job->failed || job->completed || job->page_open ||
        job->pages>=64 || width<1 || width>LEITZ_MAX_WIDTH ||
        !raster_rows || total_rows>LEITZ_MAX_ROWS ||
        top>total_rows || raster_rows>total_rows-top ||
        bottom!=total_rows-top-raster_rows ||
        (dpi_y!=300 && dpi_y!=600) || length_code>65535) return -1;
    /* One loaded cassette and one physical print width per job. The same
     * geometry and resolution policy applies to BOTH wire formats.
     * Only label length may vary on continuous media. */
    if (job->pages && (width!=job->first_width || dpi_y!=job->first_dpi))
        return -1;
    if (job->pages==0) {
        job->first_width=width;
        job->first_dpi=dpi_y;
    }
    job->width=width;
    job->remaining=raster_rows;
    job->bottom=bottom;
    if (job->protocol==LEITZ_PROTOCOL_CURRENT) {
        if (leitz_begin_media(&job->wire.current,job->out,width,total_rows,
                              raster_rows,top,bottom,length_code,dpi_y)) goto fail;
    } else {
        leitz_native_reference *n=&job->wire.native;
        const uint8_t empty[120]={0};
        const unsigned packed=(width+7)/8;
        if (leitz_native_begin_page(n,job->out,width,total_rows,dpi_y,
                                     length_code,job->pages==0)) goto fail;
        for(unsigned i=0;i<top;i++)
            if(leitz_native_row(n,empty,packed)) goto fail;
    }
    job->page_open=1;
    return 0;
fail:
    job->failed=1;
    return -1;
}

int leitz_job_row(leitz_job *job, const uint8_t *pixels) {
    if (!job || job->failed || !job->page_open || !pixels || !job->remaining)
        return -1;
    int rc=0;
    if (job->protocol==LEITZ_PROTOCOL_CURRENT)
        rc=leitz_row_media(&job->wire.current,pixels);
    else {
        uint8_t packed[120]={0};
        for(unsigned x=0;x<job->width;x++)
            if(pixels[x]) packed[x/8]|=(uint8_t)(1u<<(7-(x&7)));
        rc=leitz_native_row(&job->wire.native,packed,(job->width+7)/8);
    }
    if(rc) {job->failed=1;return -1;}
    job->remaining--;
    return 0;
}

int leitz_job_end_page(leitz_job *job) {
    if (!job || job->failed || !job->page_open || job->remaining) return -1;
    int rc=0;
    if (job->protocol==LEITZ_PROTOCOL_CURRENT)
        rc=leitz_end_page(&job->wire.current,job->cut_each_page);
    else {
        /* A native page is composed of TOP + raster rows + BOTTOM.
         * Keep page geometry in this protocol-neutral coordinator.
         * The native encoder itself controls whether accumulated trailing
         * blank lines become USB feed commands. */
        leitz_native_reference *n=&job->wire.native;
        const uint8_t empty[120]={0};
        const unsigned packed=(job->width+7)/8;
        for(unsigned i=0;i<job->bottom;i++)
            if(leitz_native_row(n,empty,packed)){job->failed=1;return -1;}
        rc=leitz_native_finish_page(n,job->cut_each_page);
    }
    if(rc){job->failed=1;return -1;}
    job->page_open=0;
    job->pages++;
    return 0;
}

int leitz_job_finish(leitz_job *job) {
    if(!job || job->failed || job->page_open || !job->pages || job->completed)
        return -1;
    /* Common cutter policy: never leave the final label uncut.
     * Command bytes remain private to the selected wire-format codec. */
    if(!job->cut_each_page) {
        int rc=job->protocol==LEITZ_PROTOCOL_CURRENT
              ?leitz_finish_job(&job->wire.current)
              :leitz_native_finish_job(&job->wire.native);
        if(rc){job->failed=1;return -1;}
    }
    job->completed=1;
    return 0;
}

int leitz_job_validate(leitz_job *job, char *reason, size_t reason_size) {
    if(!job || !job->completed || job->failed) return -1;
    if(fseek(job->out,0,SEEK_SET)) return -1;
    unsigned pages=0,validated_width=0;
    if(job->protocol==LEITZ_PROTOCOL_CURRENT) {
        if(leitz_validate(job->out,&pages,&validated_width,reason,reason_size)) return -1;
    } else {
        if(leitz_native_validate_job(job->out,&pages,&validated_width,reason,reason_size))
            return -1;
    }
    return pages==job->pages && validated_width==job->first_width ? 0 : -1;
}
unsigned leitz_job_page_count(const leitz_job *job) {return job?job->pages:0;}
const char *leitz_job_protocol_name(const leitz_job *job) {
    return job && job->protocol==LEITZ_PROTOCOL_NATIVE_LINUX_103
           ? "NativeLinux103" : "Current";
}
void leitz_job_destroy(leitz_job *job) {free(job);}
