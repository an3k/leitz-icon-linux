#ifndef CLASSIC_COMMAND_WRITE_DATA_H
#define CLASSIC_COMMAND_WRITE_DATA_H
#include <stddef.h>
#include <sys/types.h>

/* Original command2leitz _WriteData 0x100001a80..0x100001c14.
 * The original function's source and global ABI are not available.
 * This structure only exposes the recovered sink and return decisions.
 */
typedef struct {
    void *context;
    int ipp_selected;
    int socket_fd;
    int dump_present;
    int stdout_present;
    ssize_t (*send_once)(void *,int,const void *,size_t);
    size_t (*dump_once)(void *,const void *,size_t);
    size_t (*stdout_once)(void *,const void *,size_t);
} classic_command_output;
/* Original success is based on write result >0, not complete byte count. */
int classic_command_write_data(const classic_command_output *,
                               const void *,size_t);
#endif
