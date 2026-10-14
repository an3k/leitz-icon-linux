/*
 * Original-derived macOS 5.1.465 classic raster2printer CUPS candidate.
 *
 * Connects the already reconstructed OEM _main/_raster2printer,
 * _StartDocument, _DitherPage, _StartPage, all four _ProcessPage choices,
 * _EndPage, _EndDocument, original CUPS/PPD parsing, and OEM write/PRN
 * behavior to Apple's real CUPS API. The connections are analysis-level
 * C callbacks, NOT the original x86_64 C++ calling convention.
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
#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <fcntl.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* Symbols expected by the independently reconstructed OEM _main. */
extern int terminate_flag;
extern unsigned int job_size;
unsigned int job_size;

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
    int stdout_enabled;
    int socket_fd;
} classic_cups_context;
static classic_cups_context candidate={0};

void OpenLogFile(void) {
    classic_original_log_open(&candidate.log);
}
void CloseLogFile(void) {
    classic_original_log_close(&candidate.log);
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
    classic_cups_context *c=ctx;
    classic_original_prn_open(&c->prn);
}
static void select_stdout(void *ctx,int enabled) {
    classic_cups_context *c=ctx;
    c->stdout_enabled=enabled;
}
static ssize_t native_send(void *ctx,const void *bytes,size_t size) {
    classic_cups_context *c=ctx;
    return send(c->socket_fd,bytes,size,0);
}
static size_t write_stdout(void *ctx,const void *bytes,size_t size) {
    (void)ctx;
    return fwrite(bytes,size,1,stdout);
}
static size_t write_dump(void *ctx,const void *bytes,size_t size) {
    classic_cups_context *c=ctx;
    return fwrite(bytes,size,1,c->prn.dump);
}
static int write_data(void *ctx,const uint8_t *bytes,size_t size,
                      const void *descriptor,size_t descriptor_size,
                      int socket_fd) {
    classic_cups_context *c=ctx;
    /* _WriteData reads scheme at OEM descriptor +0x50, and selects
     * one send(fd,bytes,size,0) only if it contains "ipp" and fd!=-1. */
    if(descriptor_size!=CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE) return 0;
    const char *scheme=(const char *)descriptor+0x50;
    c->socket_fd=socket_fd;
    classic_write_data_io io={
        c, socket_fd!=-1 && strstr(scheme,"ipp")!=NULL,
        c->stdout_enabled,c->prn.dump!=NULL,
        native_send,write_stdout,write_dump
    };
    return classic_write_data_core(&io,bytes,size);
}
static void flush_data_streams(void *ctx) {
    classic_cups_context *c=ctx;
    (void)fflush(stdout);
    if(c->prn.dump) (void)fflush(c->prn.dump);
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
    classic_cups_context *c=ctx;
    classic_original_prn_close(&c->prn);
}
static int send_prn(void *ctx) {
    classic_cups_context *c=ctx;
    return classic_original_prn_send(&c->prn,stdout);
}
static void start_page(void *ctx,classic_oem_read_descriptor *descriptor) {
    classic_cups_context *c=ctx;
    classic_original_start_page(&c->page_io,descriptor);
}
static int process_page(void *ctx,classic_oem_read_descriptor *descriptor,
                        const uint8_t *pixels,unsigned page_index,int reprint) {
    classic_cups_context *c=ctx;
    return classic_original_process_page_callback(
        &c->process,descriptor,pixels,page_index,reprint);
}
static void end_page(void *ctx,const void *descriptor,size_t descriptor_size) {
    classic_cups_context *c=ctx;
    if(descriptor_size==CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE)
        classic_original_end_page(&c->page_io,descriptor);
}
int raster2printer(int argc,char **argv) {
    classic_cups_context *c=&candidate;
    c->socket_fd=-1; /* native original startup state, no speculative connect */
    c->stdout_enabled=1;

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
        open_debug,select_stdout,classic_original_bridge_start_document
    };
    classic_oem_page_output_io pages={
        c,classic_original_raster_dither_bridge,
        start_page,process_page,end_page
    };
    classic_original_raster_finish_ops finish={
        c,classic_original_bridge_end_document,close_fd,close_debug,
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
