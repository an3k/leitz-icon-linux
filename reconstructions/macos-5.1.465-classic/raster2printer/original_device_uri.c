#include "original_device_uri.h"
#include <cups/cups.h>
#include <cups/backend.h>
#include <cups/http.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/* Original static globals; they are deliberately not silently reset
 * between calls. Source: OEM Mach-O _ResolveDeviceURI and
 * _CheckPrinterStatus, 0x100009a30..0x100009fd3. */
int classic_original_socket=-1;
const char *classic_original_resolved_uri=NULL;

int classic_original_resolve_device_uri(
        classic_original_raster_descriptor *d) {
    char *uri=getenv("DEVICE_URI");
    int result=0;
    fprintf(stderr,"\nraster2printer.c:1318:ResolveDeviceURI(): device uri %s",uri);
    classic_original_resolved_uri=cupsBackendDeviceURI(&uri);
    fprintf(stderr,"\nraster2printer.c:1322:ResolveDeviceURI(): resolved uri %s",
            classic_original_resolved_uri);
    if (classic_original_resolved_uri) {
        /* OEM passes exactly 0 (no decoding flags), four 1024-byte
         * buffers and its descriptor port at offset 0x1098. */
        http_uri_status_t status=httpSeparateURI(
            HTTP_URI_CODING_NONE,classic_original_resolved_uri,
            d->scheme,1024,d->username,1024,d->hostname,1024,
            &d->port,d->resource,1024);
        if(status<HTTP_URI_STATUS_OK) {
            fprintf(stderr,"ERROR: Bad printer URI \"%s\".\n",
                    classic_original_resolved_uri);
        } else {
            fprintf(stderr,"hostName = %s",d->hostname);
            fprintf(stderr,"scheme = %s",d->scheme);
            fprintf(stderr,"resource = %s",d->resource);
            result=1;
            if(strstr(d->scheme,"ipp") && classic_original_resolved_uri) {
                int err=0;
                /* OEM overwrites the URI port with 0x238c/9100 as
                 * the TCP destination even if URI specified another. */
                int socket_port=0x238c;
                classic_original_socket=socket(AF_INET,SOCK_STREAM,0);
                if(classic_original_socket<0) {
                    err=errno;
                    fprintf(stderr,
                            "\nraster2printer.c:1346:ResolveDeviceURI(): create sock = %i err = %i",
                            classic_original_socket,err);
                } else if(!err) {
                    struct sockaddr_in addr;
                    memset(&addr,0,sizeof addr);
                    addr.sin_family=AF_INET;
                    addr.sin_port=htons((uint16_t)socket_port);
                    struct hostent *h=gethostbyname(d->hostname);
                    if(!h) herror("resolv");
                    /* Original unconditionally dereferences h after
                     * herror, even if lookup failed. Preserve that
                     * manufacturer defect; no invented recovery path. */
                    struct in_addr ip;
                    memcpy(&ip,h->h_addr_list[0],sizeof ip);
                    const char *text_address=inet_ntoa(ip);
                    if(text_address) addr.sin_addr.s_addr=inet_addr(text_address);
                    err=connect(classic_original_socket,
                                (struct sockaddr*)&addr,sizeof addr);
                    if(err<0) {
                        err=errno;
                        fprintf(stderr,
                                "\nraster2printer.c:1378:ResolveDeviceURI(): connect err = %i",
                                err);
                    }
                }
            }
        }
    }
    return result;
}
