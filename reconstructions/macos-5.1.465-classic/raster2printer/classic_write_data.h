#ifndef CLASSIC_WRITE_DATA_H
#define CLASSIC_WRITE_DATA_H
#include <stddef.h>
#include <sys/types.h>

/* Recovered _WriteData sink actions for classic raster2printer.
 * Observed address 0x100001450 and related branch locations.
 * The transport selector is explicitly supplied by this offline ABI;
 * the manufacturer's global descriptor and URI lookup logic are
 * NOT fully reconstructed. */
typedef struct {
  void *context;
  int socket_selected;
  int stdout_enabled;
  int dump_enabled;
  ssize_t (*send_socket)(void *,const void *,size_t);
  size_t (*write_stdout)(void *,const void *,size_t);
  size_t (*write_dump)(void *,const void *,size_t);
} classic_write_data_io;

/* Returns 1 when the selected primary write returned >0, otherwise 0.
 * This is an observational helper, NOT a claim about the original C
 * function's return ABI or all error-message side effects.
 * Dump failures do not change the primary branch's decision, and a
 * positive but short socket send is not retried.
 */
int classic_write_data_core(const classic_write_data_io *io,
                            const void *data, size_t size);
#endif
