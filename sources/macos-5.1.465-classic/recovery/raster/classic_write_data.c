#include "classic_write_data.h"

/* Original raster2printer _WriteData at 0x100001450:
 *  - the IPP-selected path invokes send() once;
 *  - stdout path invokes __fd_dump fwrite first if present;
 *  - absent __fd_stdout means no normal stream write (PrintThroughPRN);
 *  - positive short socket writes count as successful.
 * The conditional selection is an input here because the original's
 * complete global pointer/URI selection is not yet reconstructed.
 */
int classic_write_data_core(const classic_write_data_io *io,
                            const void *data, size_t size) {
  if (io->socket_selected) {
    return io->send_socket(io->context,data,size)>0;
  }
  if (io->dump_enabled) {
    /* The original does not make a dump fwrite error fatal. */
    io->write_dump(io->context,data,size);
  }
  if (!io->stdout_enabled) return 0;
  return io->write_stdout(io->context,data,size)>0;
}
