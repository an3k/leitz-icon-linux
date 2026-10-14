/* Reconstructed from original Esselte Leitz Icon 5.1.465 command2leitz
 * x86_64 Mach-O SHA-256:
 * 6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee
 * _OpenDebugFile 0x100001c90..0x100001f30
 * _OpenLogFile 0x100001f40..0x100002167
 * _SendDebugFileToPrinter 0x100002170..0x10000243c
 * _WriteDebugBuffer 0x1000024c0..0x100002518
 *
 * Original behavior including fixed system paths, unchecked sprintf,
 * ignoring chmod/IO errors and byte-wise PRN transfer is retained.
 */
#include "original_command_file_helpers.h"
#include "classic_command_original_utils.h"
#include <string.h>
#include <sys/stat.h>

#if defined(__APPLE__)
FILE *__fd_dump __asm__("__fd_dump") = NULL;
FILE *__fd_log __asm__("__fd_log") = NULL;
FILE *__fd_stdout __asm__("__fd_stdout") = NULL;
#else
FILE *__fd_dump = NULL;
FILE *__fd_log = NULL;
FILE *__fd_stdout = NULL;
#endif
char dbgprn_filename[1024];
char log_filename[1024];

void OpenDebugFile(void) {
    if (__fd_dump != NULL) {
        fprintf(stderr, "\n%s:%d:%s(): %s",
                "raster_c_functions.c",88,"OpenDebugFile",
                "debug file is open already");
        return;
    }
    snprintf(dbgprn_filename,1024,"/Library/Caches/%s","DEBUG");
    if (mkdir(dbgprn_filename,0755)==0) chmod(dbgprn_filename,0755);
    sprintf(dbgprn_filename+strlen(dbgprn_filename),"/dbg.prn");
    __fd_dump=fopen(dbgprn_filename,"wb");
    chmod(dbgprn_filename,0644);
    if (__fd_dump) {
        fprintf(stderr,"\n%s:%d:%s(): log file: '%s'",
                "raster_c_functions.c",104,"OpenDebugFile",dbgprn_filename);
    } else {
        fprintf(stderr,"\n%s:%d:%s(): %s",
                "raster_c_functions.c",108,"OpenDebugFile",
                "cannot open log file");
    }
}

void OpenLogFile(void) {
    if (__fd_log != NULL) {
        fprintf(stderr,"\n%s:%d:%s(): %s",
                "raster_c_functions.c",116,"OpenLogFile",
                "log file is open already");
        return;
    }
    snprintf(log_filename,1024,"/Library/Caches/%s","DEBUG");
    if (mkdir(log_filename,0755)==0) chmod(log_filename,0755);
    sprintf(log_filename+strlen(log_filename),"/LeitzDriverLog.log");
    __fd_log=fopen(log_filename,"w");
    chmod(log_filename,0644);
    if (__fd_log) {
        fprintf(stderr,"\n%s:%d:%s(): log file: '%s'",
                "raster_c_functions.c",132,"OpenLogFile",log_filename);
    } else {
        fprintf(stderr,"cannot open log file");
    }
}

int SendDebugFileToPrinter(void) {
    fprintf(stderr,"\n%s:%d:%s(): %s",
            "raster_c_functions.c",142,"SendDebugFileToPrinter",
            "send debug file to printer ");
    FILE *source=fopen(dbgprn_filename,"r");
    if (source) {
        int count=0;
        int ch;
        while ((ch=fgetc(source))!=EOF) {
            if (fputc(ch,stdout)==EOF) {
                fprintf(stderr,"\n%s:%d:%s(): %s",
                        "raster_c_functions.c",152,"SendDebugFileToPrinter",
                        "can not write a symbol into stdout");
                break;
            }
            ++count;
        }
        fclose(source);
        source=NULL;
        fprintf(stderr,"\n%s:%d:%s(): %d number of bytes to stdout written",
                "raster_c_functions.c",158,"SendDebugFileToPrinter",count);
    } else {
        fprintf(stderr,"\n%s:%d:%s(): cannot open prn file '%s'",
                "raster_c_functions.c",162,"SendDebugFileToPrinter",dbgprn_filename);
    }
    return 0;
}

void WriteDebugBuffer(const unsigned char *data,size_t length) {
    if (__fd_dump && data && length) {
        fwrite(data,length,1,__fd_dump);
    }
}

/* The four native global functions below were already recovered as
 * isolated original-semantic helpers. Connect the genuine manufacturer
 * global FILE* slots without changing their effects or error handling.
 * Verified original command2leitz disassembly:
 * _FlushDataStreams 0x100001c20..0x100001c5c,
 * _is_bit_set       0x100001c60..0x100001c86,
 * _CloseDebugFile   0x100002440..0x100002475,
 * _CloseLogFile     0x100002480..0x1000024b5.
 */
void FlushDataStreams(void) {
    classic_command_original_flush(stdout,__fd_dump);
}
int is_bit_set(int value,int bit) {
    return classic_command_original_is_bit_set((unsigned)value,(unsigned)bit);
}
void CloseDebugFile(void) {
    classic_command_original_close_debug(&__fd_dump);
}
void CloseLogFile(void) {
    classic_command_original_close_log(&__fd_log);
}
