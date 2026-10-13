#define _POSIX_C_SOURCE 200809L
#include "original_raster_private_helpers.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(void) {
    char text[]="ABC-BC";
    assert(classic_strrchr_wrapper(text,'B')==text+4);
    assert(classic_strrchr_wrapper(text,'Z')==NULL);
    assert(classic_strrchr_wrapper(text,'\0')==text+6);
    assert(classic_strstr_wrapper(text,"BC")==text+1);
    assert(classic_strstr_wrapper(text,"NONE")==NULL);
    assert(classic_strstr_wrapper(text,"")==text);
    for(unsigned i=0;i<65536;++i) {
        unsigned expect=((i&255u)<<8) | (i>>8);
        assert(classic_os_swap_int16((uint16_t)i)==(uint16_t)expect);
    }
    FILE *f=tmpfile();assert(f);
    int old=dup(fileno(stderr));assert(old>=0);
    assert(dup2(fileno(f),fileno(stderr))>=0);
    terminate_flag=0;
    signal_handler(15);
    assert(terminate_flag==1);
    assert(fflush(stderr)==0);
    assert(dup2(old,fileno(stderr))>=0);assert(close(old)==0);
    rewind(f);char buffer[80]={0};size_t n=fread(buffer,1,79,f);buffer[n]=0;
    assert(strcmp(buffer,"\nINFO: Cancelling the job...")==0);
    fclose(f);
    puts("Original classic signal/strrchr/strstr/OSSwapInt16 tests passed");
    return 0;
}
