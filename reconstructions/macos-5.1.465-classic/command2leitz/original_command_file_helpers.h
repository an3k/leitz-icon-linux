#ifndef ORIGINAL_COMMAND_FILE_HELPERS_H
#define ORIGINAL_COMMAND_FILE_HELPERS_H
#include <stdio.h>
#include <stddef.h>

/* macOS classic 5.1.465 command2leitz global storage and OEM functions.
 * This independent reconstruction is NOT linked into production filters. */
extern FILE *__fd_dump;
extern FILE *__fd_log;
extern char dbgprn_filename[1024];
extern char log_filename[1024];
void OpenDebugFile(void);
void OpenLogFile(void);
int SendDebugFileToPrinter(void);
void WriteDebugBuffer(const unsigned char *data, size_t length);
/* Native-original call surface using original global stream pointers.
 * The defined body operations reuse existing recovered OEM helpers. */
void FlushDataStreams(void);
int is_bit_set(int data,int bit);
void CloseDebugFile(void);
void CloseLogFile(void);
#endif
