/* Manufacturer-derived _ResolveDeviceURI execution without any socket:
 * a non-IPP URI intentionally cannot enter the OEM TCP branch. */
#include "original_device_uri.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    classic_original_raster_descriptor d={0};
    assert(setenv("DEVICE_URI","usb://printer.invalid/queue",1)==0);
    int resolved=classic_original_resolve_device_uri(&d);
    assert(resolved==1);
    assert(strcmp(d.scheme,"usb")==0);
    assert(sizeof d==0x10a8);
    assert(offsetof(classic_original_raster_descriptor,uri_tail.port)==0x1098);
    assert(classic_original_socket==-1);
    puts("original macOS URI parser, descriptor offsets and non-IPP branch passed");
    return 0;
}
