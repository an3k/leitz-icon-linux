#include "original_log_helpers.h"
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

/* From direct manufacturer disassembly, not from enhanced Linux sources. */
int classic_original_is_bit_set(int data,int index) {
  /* x86 shll %cl truncates shift count to five low-order bits. */
  uint32_t mask=1u<<((unsigned)index & 31u);
  return (((uint32_t)data & mask)!=0u);
}

void classic_original_log_open(classic_original_log_state *state) {
  if(state->log_file) {
    fprintf(stderr,"\n%s:%d:%s(): %s","raster_c_functions.c",116,
            "OpenLogFile","log file is open already");
    return;
  }
  snprintf(state->filename,1024,"/Library/Caches/%s","DEBUG");
  if(mkdir(state->filename,0755)==0) chmod(state->filename,0755);
  sprintf(state->filename+strlen(state->filename),"/LeitzDriverLog.log");
  state->log_file=fopen(state->filename,"w");
  chmod(state->filename,0644);
  if(state->log_file) {
    fprintf(stderr,"\n%s:%d:%s(): log file: '%s'",
            "raster_c_functions.c",132,"OpenLogFile",state->filename);
  } else {
    fprintf(stderr,"cannot open log file");
  }
}

void classic_original_log_close(classic_original_log_state *state) {
  if(state->log_file) {
    fclose(state->log_file);
    state->log_file=NULL;
  }
}
