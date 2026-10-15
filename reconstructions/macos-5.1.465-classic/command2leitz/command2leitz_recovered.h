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
  /* Source-level callback boundary only: the original native GetStatus
   * calls _EndOfJob(bool) directly on a successful backchannel reply.
   * The joined CUPS candidate uses the recovered native function here;
   * earlier isolated callbacks preserve their original standalone path
   * by setting this optional slot to NULL. */
  void (*end_job)(void *context, int should_send_esc_a);
} classic_command_io;

/* The four original macOS __DATA,__data status-domain and prefix
 * pointer variables are file-local, so source-level getters allow the
 * joined CUPS implementation to reuse those same OEM pointer objects.
 * These getter functions do not represent additional OEM exports. */
const char *classic_original_printer_status_prefix(void);
const char *classic_original_united_status_prefix(void);
const char *classic_original_firmware_version_prefix(void);
const char *classic_original_notification_domain(void);

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
/* These are intentionally the isolated callback implementations, NOT
 * the manufacturer's four native external function ABIs. The native
 * command candidate exports _GetStatus/_GetPrinterStatus/etc separately.
 * Apple assembler labels keep the two distinct sets from colliding. */
#if defined(__APPLE__)
int GetStatus(const classic_command_io *,char,const char *,int,double)
    __asm__("_classic_command_callback_GetStatus");
int GetPrinterStatus(const classic_command_io *,int,double)
    __asm__("_classic_command_callback_GetPrinterStatus");
int GetUnitedStatus(const classic_command_io *,int,double)
    __asm__("_classic_command_callback_GetUnitedStatus");
int GetFirmvareVersion(const classic_command_io *,int,double)
    __asm__("_classic_command_callback_GetFirmvareVersion");
#else
int GetStatus(const classic_command_io *,char,const char *,int,double);
int GetPrinterStatus(const classic_command_io *,int,double);
int GetUnitedStatus(const classic_command_io *,int,double);
int GetFirmvareVersion(const classic_command_io *,int,double);
#endif
/* The manufacturer exports _EndOfJob(bool). The callback-only
 * implementation remains available under a separate name on Apple. */
#if defined(__APPLE__)
void EndOfJob(const classic_command_io *,int)
    __asm__("_classic_command_callback_EndOfJob");
#else
void EndOfJob(const classic_command_io *,int);
#endif

/* Recovered from _main, 0x100000fe2..0x10000118f.
 * Each call consumes a command-file name/value pair. Return value 1 means
 * that the vendor's _main returns immediately with exit code 1; 0 continues.
 */
int classic_command_dispatch(const classic_command_io *io, const char *name,
                             const char *value, int should_send_esc_a);
#endif
