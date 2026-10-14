#include "original_prn_helpers.h"
#include <string.h>
#include <sys/stat.h>

/* OEM logs point to basename "raster_c_functions.c". It uses no
 * configured application logging path in this helper. */
static const char original_file[]="raster_c_functions.c";

void classic_original_prn_open(classic_original_prn_state *state) {
  if(state->dump) {
    fprintf(stderr,"\n%s:%d:%s(): %s",
            original_file,88,"OpenDebugFile","debug file is open already");
    return;
  }
  /* The OEM first formats the directory using snprintf_chk(1024),
   * mkdir(0755), and chmod(0755) ONLY when mkdir returns zero.
   * It appends "/dbg.prn" with sprintf_chk before opening.
   */
  snprintf(state->filename,1024,"/Library/Caches/%s","DEBUG");
  if(mkdir(state->filename,0755)==0) chmod(state->filename,0755);
  sprintf(state->filename+strlen(state->filename),"/dbg.prn");
  state->dump=fopen(state->filename,"wb");
  chmod(state->filename,0644);
  if(state->dump) {
    fprintf(stderr,"\n%s:%d:%s(): log file: '%s'",
            original_file,104,"OpenDebugFile",state->filename);
  } else {
    fprintf(stderr,"\n%s:%d:%s(): %s",
            original_file,108,"OpenDebugFile","cannot open log file");
  }
}

void classic_original_raster_flush_streams(
    const classic_original_prn_state *state,FILE *stdout_stream) {
  /* _FlushDataStreams: ignored flush results are manufacturer behavior. */
  (void)fflush(stdout_stream);
  if(state->dump) (void)fflush(state->dump);
}

void classic_original_prn_close(classic_original_prn_state *state) {
  /* _CloseDebugFile 0x100001e10..0x100001e45, directly checked
   * against original machine instructions: fclose then clear pointer. */
  if(state->dump) {
    fclose(state->dump);
    state->dump=NULL;
  }
}

void classic_original_prn_write_buffer(classic_original_prn_state *state,
                                       const void *buffer,size_t size) {
  /* Original _WriteDebugBuffer: three independent nonzero checks.
   * Its fwrite(ptr,size,1,stream) return is ignored. */
  if(state->dump && buffer && size) fwrite(buffer,size,1,state->dump);
}

int classic_original_prn_send(classic_original_prn_state *state,FILE *output) {
  fprintf(stderr,"\n%s:%d:%s(): %s",original_file,142,
          "SendDebugFileToPrinter","send debug file to printer ");
  FILE *source=fopen(state->filename,"r");
  if(source) {
    int amount=0;
    for(;;) {
      int ch=fgetc(source);
      if(ch==EOF) break;
      if(fputc(ch,output)==EOF) {
        fprintf(stderr,"\n%s:%d:%s(): %s",original_file,152,
                "SendDebugFileToPrinter",
                "can not write a symbol into stdout");
        break;
      }
      ++amount;
    }
    fclose(source);
    fprintf(stderr,"\n%s:%d:%s(): %d number of bytes to stdout written",
            original_file,158,"SendDebugFileToPrinter",amount);
  } else {
    fprintf(stderr,"\n%s:%d:%s(): cannot open prn file '%s'",
            original_file,162,"SendDebugFileToPrinter",state->filename);
  }
  return 0;
}
