#define _POSIX_C_SOURCE 200809L
#include "original_command_file_helpers.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Exercised only existing-open and temporary-file branches. Original
 * OpenDebugFile/OpenLogFile when closed use fixed /Library/Caches paths;
 * the test deliberately does NOT invoke those filesystem-mutating paths. */
int main(void) {
    FILE *captured_errors=tmpfile(); assert(captured_errors);
    int olderr=dup(fileno(stderr));assert(olderr>=0);
    assert(dup2(fileno(captured_errors),fileno(stderr))>=0);
    __fd_dump=tmpfile(); __fd_log=tmpfile();
    assert(__fd_dump && __fd_log);
    OpenDebugFile(); OpenLogFile();
    assert(__fd_dump && __fd_log);
    const unsigned char contents[]={0,'\n',0x80,0xff,1};
    WriteDebugBuffer(contents,sizeof(contents));
    assert(fflush(__fd_dump)==0);
    rewind(__fd_dump);
    unsigned char observed[5];
    assert(fread(observed,1,5,__fd_dump)==5);
    assert(memcmp(contents,observed,5)==0);
    WriteDebugBuffer(NULL,sizeof contents);
    WriteDebugBuffer(contents,0);
    assert(fclose(__fd_dump)==0);__fd_dump=NULL;
    assert(fclose(__fd_log)==0);__fd_log=NULL;
    char filename[]="/tmp/leitz-prn-reconstruction-XXXXXX";
    int fd=mkstemp(filename);assert(fd>=0);
    assert(write(fd,contents,sizeof contents)==(ssize_t)sizeof contents);
    assert(close(fd)==0);
    assert(snprintf(dbgprn_filename,sizeof dbgprn_filename,"%s",filename)>0);
    FILE *captured_out=tmpfile();assert(captured_out);
    int oldout=dup(fileno(stdout));assert(oldout>=0);
    assert(dup2(fileno(captured_out),fileno(stdout))>=0);
    assert(SendDebugFileToPrinter()==0);
    assert(fflush(stdout)==0);
    assert(dup2(oldout,fileno(stdout))>=0); assert(close(oldout)==0);
    rewind(captured_out);assert(fread(observed,1,5,captured_out)==5);
    assert(memcmp(contents,observed,5)==0);assert(fclose(captured_out)==0);
    assert(unlink(filename)==0);
    assert(SendDebugFileToPrinter()==0); /* OEM fopen-failure message */
    assert(fflush(stderr)==0);
    assert(dup2(olderr,fileno(stderr))>=0);assert(close(olderr)==0);
    rewind(captured_errors);
    char err[1500]={0};size_t n=fread(err,1,sizeof(err)-1,captured_errors);err[n]=0;
    assert(fclose(captured_errors)==0);
    assert(strstr(err,"raster_c_functions.c:88:OpenDebugFile(): debug file is open already"));
    assert(strstr(err,"raster_c_functions.c:116:OpenLogFile(): log file is open already"));
    assert(strstr(err,"raster_c_functions.c:142:SendDebugFileToPrinter(): send debug file to printer "));
    assert(strstr(err,"raster_c_functions.c:158:SendDebugFileToPrinter(): 5 number of bytes to stdout written"));
    assert(strstr(err,"raster_c_functions.c:162:SendDebugFileToPrinter(): cannot open prn file"));
    puts("Original command2leitz file-helper behavior checks passed");
    return 0;
}
