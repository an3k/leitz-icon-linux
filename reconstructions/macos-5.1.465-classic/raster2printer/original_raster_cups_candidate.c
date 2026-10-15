/*
 * Original-derived macOS 5.1.465 classic raster2printer CUPS candidate.
 *
 * Connects the already reconstructed OEM _main/_raster2printer,
 * _StartDocument, _DitherPage, _StartPage, all four _ProcessPage choices,
 * _EndPage, _EndDocument, original CUPS/PPD parsing, and OEM write/PRN
 * behavior to Apple's real CUPS API. The original-named page/document
 * boundaries use verified native signatures; the job coordinator and
 * internal callback adapters are NOT the original full C++ ABI.
 *
 * This binary is for OFFLINE comparison only. It retains unsafe original
 * conditions: modes 0/1 read four bytes beyond packed rows, and native
 * allocation/network failures are not completely recovered. Running it
 * on live hardware, or installing it as a printer driver, is unsupported.
 */
#include "original_raster_connected_job.h"
#include "original_raster_document_bridge.h"
#include "original_raster_page_lifecycle.h"
#include "original_raster_process_page_adapter.h"
#include "original_raster_dither_bridge.h"
#include "original_prn_helpers.h"
#include "original_log_helpers.h"
#include "classic_write_data.h"
#include "original_device_uri.h"
#include "original_raster_native_status_entry.h"
#include "original_raster_native_io_entry.h"
#include "original_raster_native_page_entry.h"
#include "original_raster_native_document_entry.h"
#include "original_raster_native_write_data_entry.h"
#include "original_raster_native_prn_entry.h"
#include "original_status_exchange.h"
#include "original_read_page.h"
#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <cups/backend.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
/* Apple's CUPS backend declaration is not exported by all macOS SDK
 * headers. This is the original libcups ABI already verified in the
 * existing reconstructed command2leitz CUPS candidate. */
ssize_t cupsBackChannelRead(char *,int,double);

/* Symbols expected by the independently reconstructed OEM _main. */
extern int terminate_flag;
/* OEM _CheckPrinterStatus 0x100009713..0x10000971f directly mutates
 * the original file-private 32-bit __ZL8job_size; all recovered page
 * and document functions share this single job-size counter.
 * Recover the native Mach-O name only, without changing job sizing. */
#if defined(__APPLE__)
unsigned int job_size __asm__("__ZL8job_size");
#else
unsigned int job_size;
#endif

_Static_assert(sizeof(job_size)==4,"OEM 32-bit job counter");
_Static_assert(sizeof(classic_oem_cups_option)==sizeof(cups_option_t),
               "CUPS option representation mismatch");
_Static_assert(offsetof(classic_oem_cups_option,name)==
               offsetof(cups_option_t,name),"CUPS option name mismatch");
_Static_assert(offsetof(classic_oem_cups_option,value)==
               offsetof(cups_option_t,value),"CUPS option value mismatch");

typedef struct {
    classic_original_raster_document_bridge bridge; /* MUST be first */
    classic_original_page_lifecycle_ops page_io;
    classic_original_process_page_adapter process;
    classic_original_prn_state prn;
    classic_original_log_state log;
    int socket_fd;
} classic_cups_context;
static classic_cups_context candidate={0};

/* Original _WriteData at 0x100001b28..0x100001b58 tests and passes
 * the actual OEM FILE* global __fd_stdout to fwrite. Recover that
 * native stream slot rather than storing a separate boolean. */
#if defined(__APPLE__)
FILE *classic_original_fd_stdout __asm__("__fd_stdout");
FILE *classic_original_fd_dump __asm__("__fd_dump");
FILE *classic_original_fd_log __asm__("__fd_log");
#else
FILE *classic_original_fd_stdout;
FILE *classic_original_fd_dump;
FILE *classic_original_fd_log;
#endif

/* The original native Mach-O declares two writable common DATA arrays:
 * _dbgprn_filename: 0x1000114d0..0x1000118cf (1024 bytes)
 * _log_filename:    0x1000118d0..0x100011ccf (1024 bytes).
 * Their names and contiguous sizes are from authenticated original
 * raster2printer.symbols.txt and the OEM OpenDebugFile file references,
 * not inferred from the synthetic private C context.
 *
 * Existing already-recovered file helper algorithms use callback-local
 * 1024-byte filename storage. This bridge moves exactly those bytes to
 * and from the original named globals at OEM helper boundaries; its
 * copies are reconstruction scaffolding, NOT recovered C++ source.
 */
char dbgprn_filename[1024];
char log_filename[1024];
_Static_assert(sizeof(dbgprn_filename)==1024,"OEM PRN filename global");
_Static_assert(sizeof(log_filename)==1024,"OEM log filename global");

void OpenLogFile(void) {
    candidate.log.log_file=classic_original_fd_log;
    memcpy(candidate.log.filename,log_filename,sizeof log_filename);
    classic_original_log_open(&candidate.log);
    memcpy(log_filename,candidate.log.filename,sizeof log_filename);
    classic_original_fd_log=candidate.log.log_file;
}
void CloseLogFile(void) {
    candidate.log.log_file=classic_original_fd_log;
    classic_original_log_close(&candidate.log);
    classic_original_fd_log=candidate.log.log_file;
}

/* Original Mach-O raster2printer exported helper entry points.
 * These definitions use the existing recovered native algorithms and
 * their global CUPS-candidate context; they do not claim the complete
 * original x86_64 C++ ABI or its private global-memory layout.
 *
 * Original addresses verified by the manufacturer symbol inventory:
 * _FlushDataStreams      0x1000015f0
 * _is_bit_set            0x100001630
 * _OpenDebugFile         0x100001660
 * _SendDebugFileToPrinter 0x100001b40
 * _CloseDebugFile        0x100001e10
 * _WriteDebugBuffer      0x100001e90
 */
void FlushDataStreams(void) {
    candidate.prn.dump=classic_original_fd_dump;
    classic_original_raster_flush_streams(&candidate.prn,stdout);
}
int is_bit_set(int value,int bit) {
    return classic_original_is_bit_set(value,bit);
}
void OpenDebugFile(void) {
    candidate.prn.dump=classic_original_fd_dump;
    memcpy(candidate.prn.filename,dbgprn_filename,sizeof dbgprn_filename);
    classic_original_prn_open(&candidate.prn);
    memcpy(dbgprn_filename,candidate.prn.filename,sizeof dbgprn_filename);
    classic_original_fd_dump=candidate.prn.dump;
}
void CloseDebugFile(void) {
    candidate.prn.dump=classic_original_fd_dump;
    classic_original_prn_close(&candidate.prn);
    classic_original_fd_dump=candidate.prn.dump;
}
void WriteDebugBuffer(const unsigned char *data,size_t size) {
    candidate.prn.dump=classic_original_fd_dump;
    classic_original_prn_write_buffer(&candidate.prn,data,size);
}
/* OEM 0x100001e05 xorl %eax,%eax; 0x100001e0c retq.
 * Unlike earlier void OEM-named binding, original _SendDebugFileToPrinter
 * returns the integer zero on both completed and fopen-error paths.
 * Keep the existing original PRN implementation and diagnostic order. */
int SendDebugFileToPrinter(void) {
    /* OEM _SendDebugFileToPrinter loads _dbgprn_filename again at
     * 0x100001bd1, instead of relying on a prior state snapshot. */
    memcpy(candidate.prn.filename,dbgprn_filename,sizeof dbgprn_filename);
    return classic_original_prn_send(&candidate.prn,stdout);
}

static int terminated(void *unused) {
    (void)unused;
    return terminate_flag!=0;
}
static int input_open(void *unused,const char *path,int flags) {
    (void)unused;
    return open(path,flags);
}
static void sleep_seconds(void *unused,unsigned n) {
    (void)unused;
    (void)sleep(n);
}
static void *raster_open(void *unused,int fd,int mode) {
    (void)unused;
    return cupsRasterOpen(fd,(cups_mode_t)mode);
}
static void *ppd_open(void *unused,const char *path) {
    (void)unused;
    return ppdOpenFile(path);
}
static int parse_options(void *unused,const char *text,int start,
                         classic_oem_cups_option **options) {
    (void)unused;
    return cupsParseOptions(text,start,(cups_option_t **)options);
}
static void mark_defaults(void *unused,void *ppd) {
    (void)unused;
    ppdMarkDefaults((ppd_file_t *)ppd);
}
static int mark_option(void *unused,void *ppd,const char *key,
                       const char *value) {
    (void)unused;
    return ppdMarkOption((ppd_file_t *)ppd,key,value);
}
static int mark_all_options(void *unused,void *ppd,int n,
                            classic_oem_cups_option *options) {
    (void)unused;
    return cupsMarkOptions((ppd_file_t *)ppd,n,(cups_option_t *)options);
}
static const char *marked_choice(void *unused,void *ppd,const char *key) {
    (void)unused;
    ppd_choice_t *choice=ppdFindMarkedChoice((ppd_file_t *)ppd,key);
    return choice ? choice->choice : NULL;
}
static void free_options(void *unused,int count,
                         classic_oem_cups_option *options) {
    (void)unused;
    cupsFreeOptions(count,(cups_option_t *)options);
}
static void close_ppd(void *unused,void *ppd) {
    (void)unused;
    ppdClose((ppd_file_t *)ppd);
}
static void open_debug(void *ctx) {
    (void)ctx;
    OpenDebugFile();
}
static void select_stdout(void *ctx,int enabled) {
    (void)ctx;
    classic_original_fd_stdout=enabled ? stdout : NULL;
}
static ssize_t native_send(void *ctx,const void *bytes,size_t size) {
    classic_cups_context *c=ctx;
    return send(c->socket_fd,bytes,size,0);
}
static size_t write_stdout(void *ctx,const void *bytes,size_t size) {
    (void)ctx;
    /* _WriteData tests this global for NULL before dispatching here. */
    return fwrite(bytes,size,1,classic_original_fd_stdout);
}
static size_t write_dump(void *ctx,const void *bytes,size_t size) {
    (void)ctx;
    WriteDebugBuffer(bytes,size);
    /* _WriteData ignores the dump's fwrite return (see native helper).
     * Its own write result comes from the chosen stdout/socket sink. */
    return 1;
}
/* Original raster2printer _WriteData 0x100001450 has the same
 * (bytes,length,socket,descriptor-by-value) call surface as the
 * manufacturer command helper. The original _CheckPrinterStatus
 * caller at 0x100009759 loads sock into %edx, copies 0x215 qwords
 * (0x10a8 bytes) to the stack, then calls _WriteData at 0x100009792.
 * This must not be confused with the POINTER entry of CheckPrinterStatus.
 * The already recovered stream selection and signed-return semantics
 * remain delegated to classic_write_data_core.
 */
int WriteData(const uint8_t *bytes,size_t size,int socket_fd,
              classic_oem_read_descriptor descriptor) {
    classic_cups_context *c=&candidate;
    /* _WriteData reads the exact original scheme offset +0x50. */
    const char *scheme=(const char *)&descriptor+0x50;
    c->socket_fd=socket_fd;
    classic_write_data_io io={
        c, socket_fd!=-1 && strstr(scheme,"ipp")!=NULL,
        classic_original_fd_stdout!=NULL,classic_original_fd_dump!=NULL,
        native_send,write_stdout,write_dump
    };
    return classic_write_data_core(&io,bytes,size);
}
static int write_data(void *ctx,const uint8_t *bytes,size_t size,
                      const void *descriptor,size_t descriptor_size,
                      int socket_fd) {
    (void)ctx;
    if(descriptor_size!=sizeof(classic_oem_read_descriptor))
        return 0; /* invalid analysis callback, not an OEM branch */
    classic_oem_read_descriptor copy;
    memcpy(&copy,descriptor,sizeof copy);
    return WriteData(bytes,size,socket_fd,copy);
}
static void flush_data_streams(void *ctx) {
    (void)ctx;
    FlushDataStreams();
}

/* Original macOS _ResolveDeviceURI 0x100009a30 takes the SAME
 * descriptor pointer as _CheckPrinterStatus, not a separately sized
 * analysis-only URI structure. Its recovered URI fields and TCP/9100
 * algorithm now share the same offset-checked 0x10a8-byte C11 object.
 * The OEM-named symbol and internal resolver use that one descriptor.
 * This function is dormant in the normal raster print path. */
int ResolveDeviceURI(classic_oem_read_descriptor *descriptor) {
    return classic_original_resolve_device_uri(descriptor);
}

/* Original _CheckPrinterStatus 0x100009190:
 * connect existing _CheckPrinterStatus control flow and resolver to
 * Apple's actual backchannel/socket entry points and OEM WriteData.
 * Only tested defined cases have original-semantic parity; this code
 * deliberately preserves the recovered -2 analysis marker for native
 * undefined cases. The manufacturer ABI is not recovered here.
 * The ordinary raster job does NOT call this function.
 */
typedef struct {
    classic_oem_read_descriptor *descriptor;
} classic_native_status_context;
static int status_terminated(void *unused) {
    (void)unused;
    return terminate_flag!=0;
}
static int status_socket_now(void *unused) {
    (void)unused;
    return classic_original_socket;
}
static int status_resolved_now(void *unused) {
    (void)unused;
    return classic_original_resolved_uri != NULL;
}
static int status_resolve(void *ctx) {
    classic_native_status_context *state=ctx;
    return ResolveDeviceURI(state->descriptor);
}
static ssize_t status_send(void *unused,int fd,const void *buf,size_t length,
                           int flags) {
    (void)unused;
    return send(fd,buf,length,flags);
}
static ssize_t status_recv(void *unused,int fd,void *buf,size_t length,
                           int flags) {
    (void)unused;
    return recv(fd,buf,length,flags);
}
static int status_putchar(void *unused,int ch) {
    (void)unused;
    return putchar(ch);
}
static int status_flush(void *unused) {
    (void)unused;
    return fflush(stdout);
}
static ssize_t status_backchannel(void *unused,void *buf,size_t length,
                                  double timeout) {
    (void)unused;
    return cupsBackChannelRead((char *)buf,(int)length,timeout);
}
static void status_message(void *unused,const char *line) {
    (void)unused;
    (void)fputs(line,stderr);
}
static int status_write_data(void *unused,const uint8_t *buf,size_t length,
                             const void *descriptor,size_t descriptor_size,
                             int fd) {
    (void)unused;
    return write_data(NULL,buf,length,descriptor,descriptor_size,fd);
}
static void status_sleep(void *unused,unsigned seconds) {
    (void)unused;
    (void)sleep(seconds);
}
static int status_errno(void *unused) {
    (void)unused;
    return errno;
}
/* The exported OEM _ReadPage (0x100008d20..0x10000918b) is a
 * separate, dormant entry. The actual normal raster2printer job
 * instead contains its own inline reader (with distinct malloc and
 * EOF semantics) and must continue using that original implementation.
 * No alternate allocation/retry code is introduced here.
 */
int ReadPage(classic_oem_read_descriptor *descriptor,
             cups_page_header2_t *header,unsigned char **pixels,
             int *requested_bytes,int *page_counter) {
    return classic_original_read_page(
        descriptor,header,pixels,requested_bytes,page_counter);
}

/* Manufacturer _CheckPrinterStatus takes its first parameter as an
 * original descriptor POINTER in rdi (0x1000091a9..0x1000091de).
 * Its epilogue 0x1000099f2..0x100009a1d masks the stored one-byte
 * result with 1, then zero-extends AL into EAX. Unlike the earlier
 * artificial int entry, the native contract is a boolean result.
 * The existing callback algorithm retains its separate int-valued
 * analysis-only -2 marker for genuinely undefined response branches.
 * Converting that marker here does not assert OEM error-path parity. */
_Bool CheckPrinterStatus(classic_oem_read_descriptor *descriptor) {
    classic_native_status_context state={descriptor};
    classic_original_status_io io={
        .context=&state,
        .device_scheme=(const char *)descriptor+0x50,
        .socket_fd=classic_original_socket,
        .resolved_uri_present=classic_original_resolved_uri!=NULL,
        .termination_flag=status_terminated,
        .resolve_device_uri=status_resolve,
        .send_once=status_send,
        .recv_once=status_recv,
        .put_character=status_putchar,
        .flush_stdout=status_flush,
        .read_backchannel=status_backchannel,
        .report=status_message,
        .write_data=status_write_data,
        .sleep_seconds=status_sleep,
        .job_size=&job_size,
        .last_error=status_errno,
        .descriptor=(const unsigned char *)descriptor,
        .current_socket_fd=status_socket_now,
        .resolved_uri_is_present=status_resolved_now
    };
    return classic_original_check_status(&io) != 0;
}
static void close_raster(void *unused,void *raster) {
    (void)unused;
    cupsRasterClose((cups_raster_t *)raster);
}
static int close_fd(void *unused,int fd) {
    (void)unused;
    return close(fd);
}
static void close_debug(void *ctx) {
    (void)ctx;
    CloseDebugFile();
}
static int send_prn(void *ctx) {
    (void)ctx;
    return SendDebugFileToPrinter();
}
/* Native page ABI verified by authenticated macOS 5.1.465 Mach-O
 * _raster2printer at 0x100005fbe..0x10000605e, and native
 * _StartPage 0x100007399, _EndPage 0x1000089bb. The OEM-named
 * StartPage, ProcessPage and EndPage use their original call surfaces.
 * Unmodified source-only callback adapters remain below.
 * Original StartDocument/EndDocument also accept a 0x10a8-byte
 * descriptor by value (verified OEM caller 0x100005463..0x10000548e,
 * 0x1000062eb..0x10000632d and callee rbp+0x10 entry accesses).
 * Reuse their existing, independently reconstructed algorithms.
 */
void StartDocument(classic_oem_read_descriptor descriptor) {
    classic_original_start_document(
        &candidate.bridge.original,(const unsigned char *)&descriptor);
}
void EndDocument(classic_oem_read_descriptor descriptor) {
    classic_original_end_document(
        &candidate.bridge.original,(unsigned char *)&descriptor);
}
void DitherPage(classic_oem_read_descriptor *descriptor,
                const uint8_t *rgb,uint8_t *monochrome,unsigned quality) {
    classic_original_dither_page(descriptor,rgb,monochrome,quality);
}
void StartPage(classic_oem_read_descriptor *descriptor) {
    classic_original_start_page(&candidate.page_io,descriptor);
}
int ProcessPage(classic_oem_read_descriptor *descriptor,
                const uint8_t *pixels,unsigned page_index,int reprint) {
    return classic_original_process_page_callback(
        &candidate.process,descriptor,pixels,page_index,reprint);
}
void EndPage(classic_oem_read_descriptor descriptor) {
    classic_original_end_page(&candidate.page_io,&descriptor);
}
/* The connected-job callback ABI is ours, not the original's. These
 * bridges call the actual OEM-signature exports above, rather than
 * attaching a private context argument to OEM-named symbols. */
static void connected_start_document(void *ctx,
                                     const unsigned char *descriptor,
                                     unsigned size) {
    (void)ctx;
    if(size==CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE) {
        classic_oem_read_descriptor by_value;
        memcpy(&by_value,descriptor,sizeof by_value);
        StartDocument(by_value);
    }
}
static void connected_end_document(void *ctx,const void *descriptor,
                                   size_t size) {
    (void)ctx;
    if(size==CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE) {
        classic_oem_read_descriptor by_value;
        memcpy(&by_value,descriptor,sizeof by_value);
        EndDocument(by_value);
    }
}
static void connected_dither_page(void *ctx,classic_oem_read_descriptor *d,
                                  const uint8_t *rgb,uint8_t *mono,
                                  unsigned quality) {
    (void)ctx;
    DitherPage(d,rgb,mono,quality);
}
static void connected_start_page(void *ctx,classic_oem_read_descriptor *d) {
    (void)ctx;
    StartPage(d);
}
static int connected_process_page(void *ctx,classic_oem_read_descriptor *d,
                                  const uint8_t *pixels,unsigned index,
                                  int reprint) {
    (void)ctx;
    return ProcessPage(d,pixels,index,reprint);
}
static void connected_end_page(void *ctx,const void *d,size_t size) {
    (void)ctx;
    if(size==CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE) {
        classic_oem_read_descriptor by_value;
        memcpy(&by_value,d,sizeof by_value);
        EndPage(by_value);
    }
}
int raster2printer(int argc,char **argv) {
    classic_cups_context *c=&candidate;
    c->socket_fd=-1; /* native original startup state, no speculative connect */
    classic_original_fd_stdout=stdout;

    c->bridge.original=(classic_original_document_ops){
        c,write_data,flush_data_streams,close_raster,
        &job_size,-1,terminated
    };
    c->page_io=(classic_original_page_lifecycle_ops){
        c,write_data,flush_data_streams,&job_size,-1
    };
    classic_original_process_page_adapter_initialize(
        &c->process,c->page_io,terminated,c);

    classic_oem_preflight_io preflight={
        c,input_open,sleep_seconds,raster_open,ppd_open,
        parse_options,mark_defaults,mark_option,mark_all_options
    };
    classic_oem_option_initialization_io initialization={
        preflight,marked_choice,free_options,close_ppd,
        open_debug,select_stdout,connected_start_document
    };
    classic_oem_page_output_io pages={
        c,connected_dither_page,
        connected_start_page,connected_process_page,connected_end_page
    };
    classic_original_raster_finish_ops finish={
        c,connected_end_document,close_fd,close_debug,
        send_prn,write_data
    };
    classic_original_raster_connected_io io={
        initialization,pages,finish,terminated,c,-1,&job_size
    };
    classic_oem_read_descriptor descriptor;
    classic_oem_active_page_state page;
    unsigned page_index=0;
    return classic_original_raster_connected_job(
        &io,argc,argv,&descriptor,&page,&page_index);
}
