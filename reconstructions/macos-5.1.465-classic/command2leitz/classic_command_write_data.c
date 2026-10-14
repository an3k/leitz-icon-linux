#include "classic_command_write_data.h"
#include <stdio.h>

/* Original command2leitz _WriteData 0x100001a80..0x100001c14:
 * socket choice: URI contains "ipp" AND descriptor != -1.
 * Exactly one send(,flags=0). Otherwise optional debug fwrite,
 * then one stdout fwrite if available. No retries.
 * The original compares the signed socket result ONLY with zero: even -1
 * returns success and emits no error. This is intentional OEM behavior.
 * A zero-size non-socket write returns before any sink or diagnostic.
 * When the selected primary sink returns zero, the original fprintf at
 * 0x100001b71..0x100001bf4 writes its original diagnostic to stderr.
 */
int classic_command_write_data(const classic_command_output *io,
                               const void *data,size_t size) {
    ssize_t result;
    if (io->ipp_selected && io->socket_fd != -1) {
        result=io->send_once(io->context,io->socket_fd,data,size);
    } else {
        if (size==0) return 0;
        if (io->dump_present)
            io->dump_once(io->context,data,size);
        if (!io->stdout_present) return 0;
        result=(ssize_t)io->stdout_once(io->context,data,size);
    }
    if (result==0)
        fprintf(stderr,"\nraster_c_functions.c:55:WriteData(): an error occured while writing data of size : %ld",(long)size);
    return result != 0;
}
