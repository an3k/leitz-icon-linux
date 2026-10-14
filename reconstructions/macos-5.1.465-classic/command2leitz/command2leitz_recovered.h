#ifndef COMMAND2LEITZ_RECOVERED_H
#define COMMAND2LEITZ_RECOVERED_H

#include <stddef.h>
#include <sys/types.h>

/* Isolated C reconstruction of the vendor's ACTIVE command-processing path.
 * The callbacks are research substitutes for macOS/CUPS APIs, not an exact
 * reconstruction of all the binary's separately linked helper functions.
 * No claim of a complete vendor-equivalent executable is made.
 */
typedef struct {
  void *context;
  int (*put_character)(void *context, int ch);
  int (*flush_output)(void *context);
  ssize_t (*read_backchannel)(void *context, char *buffer, int count,
                              double timeout);
  void (*message)(void *context, const char *text);
  /* Return 1 if cupsFileOpen/cupsFilePrintf/cupsFileClose succeeded;
   * otherwise 0. This is intentionally NOT a fatal condition. */
  int (*write_leitz_log)(void *context, const char *content);
  void (*post_notification)(void *context, const char *domain);
} classic_command_io;

/* Recovered from 0x100001440..0x100001a26 and 0x100001a30..0x100001a76.
 * The vendor's ShouldSend_ESC_A flag has no observable influence here.
 * A successful backchannel read sends ESC Z even if log writing fails.
 */
int classic_command_get_status(const classic_command_io *io, char command,
                               const char *prefix, int should_send_esc_a,
                               double timeout);

/* Reconstructed original exported native function boundaries.
 * The callback C argument surface is an OFFLINE scaffold; the
 * manufacturer's compiled C++ ABI is NOT asserted identical.
 * The wrappers reuse the single already recovered _GetStatus body.
 */
int GetStatus(const classic_command_io *, char command,const char *prefix,
              int should_send_esc_a,double timeout);
int GetPrinterStatus(const classic_command_io *,int,double);
int GetUnitedStatus(const classic_command_io *,int,double);
int GetFirmvareVersion(const classic_command_io *,int,double);
void EndOfJob(const classic_command_io *,int);

/* Recovered from _main, 0x100000fe2..0x10000118f.
 * Each call consumes a command-file name/value pair. Return value 1 means
 * that the vendor's _main returns immediately with exit code 1; 0 continues.
 */
int classic_command_dispatch(const classic_command_io *io, const char *name,
                             const char *value, int should_send_esc_a);
#endif
