/* Protocol-neutral page/job controller.
 * Only the protocol_ops vtable changes wire encoding. Every media, geometry,
 * raster, pagination and cutter policy applies identically to both codecs. */
#include "leitz_jobs.h"
#include <stdlib.h>
#include <string.h>

struct leitz_job {
 FILE *out;
 const leitz_protocol_ops *ops;
 void *state;
 unsigned pages,first_width,first_dpi,width,remaining;
 int cut_each_page,page_open,completed,failed,current_full_width_raw;
};

leitz_job *leitz_job_create(FILE *out,leitz_protocol protocol,int cut_each_page){
 const leitz_protocol_ops *ops=leitz_protocol_get(protocol);
 if(!out||!ops||(cut_each_page!=0&&cut_each_page!=1))return NULL;
 leitz_job *job=calloc(1,sizeof *job);
 if(!job)return NULL;
 job->state=calloc(1,ops->state_size);
 if(!job->state){free(job);return NULL;}
 job->ops=ops;job->out=out;job->cut_each_page=cut_each_page;
 return job;
}
int leitz_job_set_current_full_width_raw(leitz_job *job,int enabled){
 if(!job||enabled<0||enabled>1||job->pages||job->page_open||job->completed)return -1;
 if(enabled && strcmp(job->ops->name,"Current"))return -1;
 job->current_full_width_raw=enabled;
 return 0;
}
int leitz_job_begin_page(leitz_job *job,unsigned width,unsigned total_rows,
                         unsigned raster_rows,unsigned top,unsigned bottom,
                         unsigned length_code,unsigned dpi_y){
 if(!job||job->failed||job->completed||job->page_open||job->pages>=64||
    width<1||width>LEITZ_MAX_WIDTH||!raster_rows||
    total_rows>LEITZ_MAX_ROWS||top>total_rows||
    raster_rows>total_rows-top||bottom!=total_rows-top-raster_rows||
    (dpi_y!=300&&dpi_y!=600)||length_code>65535)return -1;
 /* A cassette is not changed inside a print job. Length is per page. */
 if(job->pages&&(width!=job->first_width||dpi_y!=job->first_dpi))return -1;
 if(!job->pages){job->first_width=width;job->first_dpi=dpi_y;}
 leitz_page page={width,total_rows,raster_rows,top,bottom,length_code,dpi_y,job->current_full_width_raw};
 if(job->ops->begin(job->state,job->out,&page,job->pages==0)){
  job->failed=1;return -1;
 }
 job->width=width;job->remaining=raster_rows;job->page_open=1;
 return 0;
}
int leitz_job_row(leitz_job *job,const uint8_t *pixels){
 if(!job||job->failed||!job->page_open||!pixels||!job->remaining)return -1;
 if(job->ops->row(job->state,pixels,job->width)){
  job->failed=1;return -1;
 }
 job->remaining--;return 0;
}
int leitz_job_end_page(leitz_job *job){
 if(!job||job->failed||!job->page_open||job->remaining)return -1;
 if(job->ops->end_page(job->state,job->cut_each_page)){
  job->failed=1;return -1;
 }
 job->page_open=0;job->pages++;return 0;
}
int leitz_job_finish(leitz_job *job){
 if(!job||job->failed||job->page_open||!job->pages||job->completed)return -1;
 if(!job->cut_each_page&&job->ops->finish(job->state)){
  job->failed=1;return -1;
 }
 job->completed=1;return 0;
}
int leitz_job_validate(leitz_job *job,char *reason,size_t reason_size){
 if(!job||!job->completed||job->failed||fseek(job->out,0,SEEK_SET))return -1;
 unsigned pages=0,width=0;
 if(job->ops->validate(job->out,&pages,&width,reason,reason_size))return -1;
 return pages==job->pages&&width==job->first_width?0:-1;
}
unsigned leitz_job_page_count(const leitz_job *job){return job?job->pages:0;}
const char *leitz_job_protocol_name(const leitz_job *job){
 return job?job->ops->name:"Unavailable";
}
void leitz_job_destroy(leitz_job *job){
 if(job){free(job->state);free(job);}
}
