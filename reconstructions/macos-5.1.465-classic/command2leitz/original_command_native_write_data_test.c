#define _POSIX_C_SOURCE 200809L
#include "original_command_native_write_data.h"
#include "original_command_file_helpers.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* No original system paths or physical printer. All bytes remain within
 * anonymous temporary streams / AF_UNIX socketpair. This validates the
 * recovered defined native function call surface and sink selection.
 * The existing authentic macOS command executable comparisons separately
 * verify that introducing the dormant native entry changes no commands. */
int main(void) {
    classic_command_native_descriptor descriptor={0};
    assert(sizeof descriptor==0x10a8u);
    const unsigned char bytes[]={0,0x1b,'U',0xfe,'Z',0};
    __fd_stdout=tmpfile();
    __fd_dump=tmpfile();
    assert(__fd_stdout && __fd_dump);
    assert(WriteData(bytes,sizeof bytes,-1,descriptor)==1);
    assert(fflush(__fd_stdout)==0 && fflush(__fd_dump)==0);
    unsigned char captured[sizeof bytes];
    rewind(__fd_stdout);
    assert(fread(captured,1,sizeof captured,__fd_stdout)==sizeof captured);
    assert(memcmp(captured,bytes,sizeof bytes)==0);
    rewind(__fd_dump);
    assert(fread(captured,1,sizeof captured,__fd_dump)==sizeof captured);
    assert(memcmp(captured,bytes,sizeof bytes)==0);

    /* A nonzero request with no primary stdout sink still writes the
     * PRN dump, then returns zero. Original does not fabricate stdout. */
    FILE *out=__fd_stdout; __fd_stdout=NULL;
    assert(WriteData(bytes,sizeof bytes,-1,descriptor)==0);
    assert(fflush(__fd_dump)==0);
    assert(fseek(__fd_dump,0,SEEK_END)==0);
    assert(ftell(__fd_dump)==2*(long)sizeof bytes);
    assert(fclose(out)==0);
    assert(fclose(__fd_dump)==0);
    __fd_dump=NULL;

    /* The OEM specifically checks "ipp" anywhere in the descriptor
     * scheme at +0x50. With a valid fd, only send() is called. */
    memcpy(descriptor.bytes+0x50,"ipp",4);
    int socketfd[2];assert(socketpair(AF_UNIX,SOCK_STREAM,0,socketfd)==0);
    assert(WriteData(bytes,sizeof bytes,socketfd[0],descriptor)==1);
    assert(recv(socketfd[1],captured,sizeof captured,0)==(ssize_t)sizeof captured);
    assert(memcmp(captured,bytes,sizeof bytes)==0);
    /* The OEM compares a negative send result != 0 and returns true,
     * without retries or a new diagnostic. */
    assert(close(socketfd[0])==0);
    assert(WriteData(bytes,sizeof bytes,socketfd[0],descriptor)==1);
    assert(close(socketfd[1])==0);

    /* An IPP scheme is not sufficient when fd==-1; zero-length
     * non-socket fallback returns zero before writing any stream. */
    assert(WriteData(bytes,0,-1,descriptor)==0);
    puts("OEM command WriteData by-value native surface and sinks passed");
    return 0;
}
