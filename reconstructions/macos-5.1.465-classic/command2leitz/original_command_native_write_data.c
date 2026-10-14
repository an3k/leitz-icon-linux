#include "original_command_native_write_data.h"
#include "original_command_file_helpers.h"
#include "classic_command_write_data.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

/* Original command2leitz _WriteData, OEM 0x100001a80..0x100001c14.
 * Use the already recovered signed-result semantics in
 * classic_command_write_data.c. Unlike the earlier C-only wrapper,
 * this exposes the original data/length/fd/stack-descriptor order.
 * The file slots are the manufacturer's actual __fd_stdout / __fd_dump
 * globals; no opening, closing or network connection is introduced. */
static ssize_t native_send_once(void *ctx,int fd,const void *data,size_t size) {
    (void)ctx;
    return send(fd,data,size,0);
}
static size_t native_dump_once(void *ctx,const void *data,size_t size) {
    (void)ctx;
    return fwrite(data,size,1,__fd_dump);
}
static size_t native_stdout_once(void *ctx,const void *data,size_t size) {
    (void)ctx;
    return fwrite(data,size,1,__fd_stdout);
}
int WriteData(const void *data,size_t size,int socket_fd,
              classic_command_native_descriptor descriptor) {
    /* Original Mach-O leaq 0x10(%rbp), %rax; addq $0x50, %rax,
     * then strstr(..., "ipp"). Socket send only when descriptor
     * contains ipp and socket_fd!=-1. */
    const char *scheme=(const char *)descriptor.bytes+0x50;
    classic_command_output io={
        NULL, strstr(scheme,"ipp")!=NULL, socket_fd,
        __fd_dump!=NULL, __fd_stdout!=NULL,
        native_send_once,native_dump_once,native_stdout_once
    };
    return classic_command_write_data(&io,data,size);
}
