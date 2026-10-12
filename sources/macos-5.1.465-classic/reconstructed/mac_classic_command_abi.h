/* ABI-only CUPS 2.x declarations for the offline research environment.
 * Production builds MUST use the platform's matching official CUPS 2.x
 * headers instead; no CUPS data structures are dereferenced here.
 * Verified exported symbols: Debian libcups.so.2 (CUPS 2.x).
 */
#ifndef MAC_CLASSIC_COMMAND_ABI_H
#define MAC_CLASSIC_COMMAND_ABI_H
#include <stddef.h>
#include <sys/types.h>
typedef struct _cups_file_s cups_file_t;
cups_file_t *cupsFileOpen(const char *filename,const char *mode);
cups_file_t *cupsFileStdin(void);
char *cupsFileGetConf(cups_file_t *fp,char *buffer,size_t size,
                      char **value,int *line_number);
int cupsFileClose(cups_file_t *fp);
ssize_t cupsBackChannelRead(char *buffer,int bytes,double timeout);
#endif
