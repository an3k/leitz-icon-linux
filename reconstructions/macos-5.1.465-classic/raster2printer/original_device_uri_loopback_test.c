/* Offline-only OEM _ResolveDeviceURI TCP branch verification.
 * The listen socket binds only 127.0.0.1:9100 on a disposable macOS
 * runner. The original manufacturer algorithm always connects to
 * port 9100 for an IPP URI, independent of the URI-supplied port.
 * This C-only test does not contact or configure a printer.
 */
/* macOS CUPS headers use BSD u_int/u_char types. Since this
 * loopback-only fixture also requests POSIX APIs, expose both sets
 * before the shared original raster descriptor includes cups/raster.h.
 * No OEM behavior or production compilation flags are changed. */
#if defined(__APPLE__)
#define _DARWIN_C_SOURCE 1
#endif
#define _POSIX_C_SOURCE 200809L
#include "original_device_uri.h"
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
    int listener=socket(AF_INET,SOCK_STREAM,0);
    assert(listener>=0);
    int yes=1;
    assert(setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes)==0);
    struct sockaddr_in local={0};
    local.sin_family=AF_INET;
    local.sin_port=htons(9100);
    assert(inet_pton(AF_INET,"127.0.0.1",&local.sin_addr)==1);
    assert(bind(listener,(struct sockaddr *)&local,sizeof local)==0);
    assert(listen(listener,1)==0);

    assert(setenv("DEVICE_URI","ipp://127.0.0.1:4231/printers/icon",1)==0);
    classic_original_raster_descriptor descriptor={0};
    assert(classic_original_resolve_device_uri(&descriptor)==1);
    assert(strcmp(descriptor.scheme,"ipp")==0);
    assert(strcmp(descriptor.hostname,"127.0.0.1")==0);
    assert(strcmp(descriptor.resource,"/printers/icon")==0);
    assert(descriptor.uri_tail.port==4231); /* OEM keeps URI port in descriptor */
    assert(classic_original_socket>=0);

    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(listener,&readset);
    struct timeval timeout={4,0};
    assert(select(listener+1,&readset,NULL,NULL,&timeout)==1);
    int incoming=accept(listener,NULL,NULL);
    assert(incoming>=0);

    struct sockaddr_in connected={0};
    socklen_t length=sizeof connected;
    assert(getpeername(classic_original_socket,
                       (struct sockaddr*)&connected,&length)==0);
    assert(connected.sin_family==AF_INET);
    assert(connected.sin_port==htons(9100));
    assert(connected.sin_addr.s_addr==local.sin_addr.s_addr);

    assert(close(incoming)==0);
    assert(close(classic_original_socket)==0);
    assert(close(listener)==0);
    puts("OEM ResolveDeviceURI loopback connection to fixed TCP/9100 passed");
    return 0;
}
