#include "original_status_gate.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
 assert(classic_original_status_uses_resolver("usb://Leitz", -1, 0)==0);
 assert(classic_original_status_uses_resolver("usb://Leitz", -1, 1)==0);
 assert(classic_original_status_uses_resolver("ipp://127.0.0.1/ipp/print", -1, 0)==1);
 assert(classic_original_status_uses_resolver("ipp://127.0.0.1/ipp/print", 4, 0)==0);
 assert(classic_original_status_uses_resolver("ipp://127.0.0.1/ipp/print", 4, 1)==1);
 assert(classic_original_status_uses_resolver("http://x/ipp", -1, 0)==1);
 assert(classic_original_status_uses_resolver("IPP://x/", -1, 0)==0);
 puts("original CheckPrinterStatus entry gate checks passed");
 return 0;
}
