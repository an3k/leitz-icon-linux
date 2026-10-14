#include "original_status_exchange.h"
#include <stdio.h>
#include <string.h>

/* Manufacturer _CheckPrinterStatus, disassembled from original macOS 5.1.465
 * Mach-O 0x100009190..0x100009a2e. Preserves the observed request paths,
 * message strings, unbounded original retry loop and 7-second CUPS timeout.
 * The original status reset's full by-value 0x10a8-byte descriptor,
 * socket argument, and send/recv flags are now represented. This remains a
 * separate callback-based reconstruction, not the vendor's native ABI;
 * its undefined cases and full OEM global device state are not recovered.
 * It is linked as a dormant named entry into the OFFLINE macOS CUPS
 * reconstruction but is not invoked by ordinary printing or installed.
 */
static int ipp_scheme(const classic_original_status_io *io) {
    return strstr(io->device_scheme,"ipp") != NULL;
}
static void report(const classic_original_status_io *io,const char *line) {
    io->report(io->context,line);
}
int classic_original_check_status(const classic_original_status_io *io) {
    int had_waiting_media=0;
    char reply[64];
    const unsigned char reset_end[4]={0x1b,'@',0x1b,'Z'};
    for (;;) {
        if (io->termination_flag(io->context) ||
            (ipp_scheme(io) && io->socket_fd == -1)) {
            if (ipp_scheme(io)) io->resolve_device_uri(io->context);
            return had_waiting_media;
        }
        report(io,"\nINFO: Update printer status...\n");
        ssize_t received;
        if (ipp_scheme(io) && io->resolved_uri_present && io->socket_fd!=-1) {
            const unsigned char req[2]={0x1b,'U'};
            ssize_t written=io->send_once(io->context,io->socket_fd,req,2,0);
            if (written==2) {
                received=io->recv_once(io->context,io->socket_fd,reply,64,0);
                if (received==-1) {
                    char line[128];
                    snprintf(line,sizeof line,
                             "\nraster2printer.c:1138:CheckPrinterStatus(): read error = %i",
                             io->last_error(io->context));
                    report(io,line);
                }
            } else {
                if (written<0) {
                    char line[128];
                    snprintf(line,sizeof line,
                             "\nraster2printer.c:1125:CheckPrinterStatus(): write error = %i",
                             io->last_error(io->context));
                    report(io,line);
                }
                /* Original reads an uninitialized received-count here:
                 * this branch has no well-defined C-level parity. */
                return -2;
            }
        } else {
            io->put_character(io->context,0x1b);
            io->put_character(io->context,'U');
            io->flush_stdout(io->context);
            received=io->read_backchannel(io->context,reply,64,7.0);
        }
        report(io,"\nSTATE: none\n");
        if (received==-1) {
            char line[128];
            snprintf(line,sizeof line,
                     "\nINFO: unable to read back channel data, errno=%d\n",
                     io->last_error(io->context));
            report(io,line);
            report(io,"\nSTATE: +com.leitz.general-error\n");
            report(io,"\nINFO: General error occurred, will retry in 5 seconds.\n");
            return had_waiting_media;
        }
        if (received==0) {
            report(io,"\nINFO: empty back channel data\n");
            report(io,"\nSTATE: +com.leitz.general-error\n");
            report(io,"\nINFO: General error occurred, will retry in 5 seconds.\n");
            return had_waiting_media;
        }
        if (received<0 || received>=64) {
            /* The OEM terminates strings only for counts 0..63; >=64
             * accesses uninitialized stack bytes in subsequent strstr. */
            return -2;
        }
        reply[received]='\0';
        if (strstr(reply,"SKU:Load Error;") || strstr(reply,"USED:-1")) {
            report(io,"\nSTATE: +com.leitz.out-of-paper-error\n");
            report(io,"\nINFO: Out of labels, will retry in 5 seconds.\n");
            *io->job_size += 4u;
            /* _CheckPrinterStatus 0x1000096d2..0x100009797:
             * copy all 0x10a8 bytes BEFORE calling _WriteData; the
             * manufacturer ignores the return, including a failed send.
             * Its original descriptor is passed by value in the ABI.
             */
            unsigned char original_copy[CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE];
            memcpy(original_copy,io->descriptor,sizeof original_copy);
            (void)io->write_data(io->context,reset_end,4,original_copy,
                                 sizeof original_copy,io->socket_fd);
            had_waiting_media=1;
            io->sleep_seconds(io->context,5);
        } else if (strstr(reply,"SKU:Door Open;")) {
            report(io,"\nSTATE: +com.leitz.door-open-error\n");
            report(io,"\nINFO: The door is open, will retry in 5 seconds.\n");
            had_waiting_media=1;
            io->sleep_seconds(io->context,5);
        } else if (strstr(reply,"SKU:No Cartridge;")) {
            report(io,"\nSTATE: +com.leitz.no-cartridge-error\n");
            report(io,"\nINFO: There is no cartridge in the printer, will retry in 5 seconds.\n");
            had_waiting_media=1;
            io->sleep_seconds(io->context,5);
        } else if (strstr(reply,"SKU:Bad Cartridge;")) {
            report(io,"\nSTATE: +com.leitz.general-error\n");
            report(io,"\nINFO: General error occurred, will retry in 5 seconds.\n");
            return had_waiting_media;
        } else {
            /* The original's intervening incompatible-cartridge branch
             * is unreachable: its condition is set immediately before it. */
            report(io,"\nSTATE: +com.leitz.ready");
            report(io,"\nINFO: The printer is ready.\n");
            return had_waiting_media;
        }
        /* No new retry limit: the OEM retries until a terminal response
         * or until termination is seen at the top of the next iteration. */
    }
}
