#define _POSIX_C_SOURCE 200809L
#include "original_prn_helpers.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
  /* Neither test path nor the state touches /Library/Caches/DEBUG. */
  classic_original_prn_state state={0};
  char file[]="/tmp/leitz-classic-prn-XXXXXX";
  int fd=mkstemp(file);
  assert(fd>=0);
  unsigned char expected[]={0x1b,0x47,0x00,0xff,0x1b,0x5a};
  assert(write(fd,expected,sizeof expected)==(ssize_t)sizeof expected);
  assert(close(fd)==0);
  strcpy(state.filename,file);
  FILE *destination=tmpfile();
  assert(destination!=NULL);
  assert(classic_original_prn_send(&state,destination)==0);
  assert(fflush(destination)==0);
  assert(fseek(destination,0,SEEK_SET)==0);
  unsigned char received[sizeof expected]={0};
  assert(fread(received,1,sizeof received,destination)==sizeof expected);
  assert(memcmp(received,expected,sizeof expected)==0);
  fclose(destination);
  assert(unlink(file)==0);
  /* OEM returns zero even if the original PRN file cannot be reopened. */
  assert(classic_original_prn_send(&state,stdout)==0);
  /* OEM _WriteDebugBuffer writes one element of the supplied size. */
  state.dump=tmpfile();
  assert(state.dump!=NULL);
  classic_original_prn_write_buffer(&state,expected,sizeof expected);
  FILE *stdout_fixture=tmpfile();
  assert(stdout_fixture!=NULL);
  assert(fwrite(expected,sizeof expected,1,stdout_fixture)==1);
  /* Check the manufacturer's two unconditional/conditional flush calls
   * through actual OS-level file sizes (not just C stdio buffer reads). */
  classic_original_raster_flush_streams(&state,stdout_fixture);
  struct stat sb={0};
  assert(fstat(fileno(stdout_fixture),&sb)==0);
  assert(sb.st_size==(off_t)sizeof expected);
  assert(fstat(fileno(state.dump),&sb)==0);
  assert(sb.st_size==(off_t)sizeof expected);
  assert(fseek(state.dump,0,SEEK_SET)==0);
  unsigned char fromdump[sizeof expected]={0};
  assert(fread(fromdump,1,sizeof fromdump,state.dump)==sizeof expected);
  assert(memcmp(fromdump,expected,sizeof expected)==0);
  classic_original_prn_close(&state);
  assert(state.dump==NULL);
  classic_original_raster_flush_streams(&state,stdout_fixture);
  assert(fclose(stdout_fixture)==0);
  /* Never invoke OpenDebugFile here: its original fixed path is global. */
  return 0;
}
