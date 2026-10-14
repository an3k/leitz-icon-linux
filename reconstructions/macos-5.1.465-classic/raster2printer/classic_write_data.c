#include "classic_write_data.h"
#include <stdio.h>

/* Original raster2printer _WriteData at 0x100001450..0x1000015e4:
 * one socket send or optional dump followed by stdout fwrite.
 * The original checks for zero, not for negative results. A negative
 * send therefore counts as success and does not produce a diagnostic.
 * Non-socket zero-length or absent-stdout returns before diagnostics.
 * Exact OEM error text and source location reconstructed from disassembly
 * 0x100001541..0x1000015c4. Sink selection is provided by this offline ABI.
 */
int classic_write_data_core(const classic_write_data_io *io,
                            const void *data, size_t size) {
  ssize_t result;
  if (io->socket_selected) {
    result=io->send_socket(io->context,data,size);
  } else {
    if (size==0) return 0;
    if (io->dump_enabled)
      io->write_dump(io->context,data,size);
    if (!io->stdout_enabled) return 0;
    result=(ssize_t)io->write_stdout(io->context,data,size);
  }
  if (result==0)
    fprintf(stderr,"\nraster_c_functions.c:55:WriteData(): an error occured while writing data of size : %ld",(long)size);
  return result!=0;
}
