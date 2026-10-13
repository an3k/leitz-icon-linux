#include "classic_command_write_data.h"

/* Original Mach-O command2leitz 0x100001a80..0x100001c14:
 * socket choice: URI contains "ipp" AND descriptor != -1.
 * Exactly one send(,flags=0). Otherwise one optional debug fwrite,
 * then one stdout fwrite if available. No retries.
 * A zero-length buffer returns failure before any sink invocation.
 * Failure diagnostics in _fprintf are not yet reconstructed here.
 */
int classic_command_write_data(const classic_command_output *io,
                               const void *data,size_t size) {
    if (io->ipp_selected && io->socket_fd != -1)
        return io->send_once(io->context,io->socket_fd,data,size)>0;
    if (size==0) return 0;
    if (io->dump_present)
        io->dump_once(io->context,data,size);
    if (!io->stdout_present) return 0;
    return io->stdout_once(io->context,data,size)>0;
}
