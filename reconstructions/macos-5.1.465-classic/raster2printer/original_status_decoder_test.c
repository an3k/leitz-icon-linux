#include "original_status_decoder.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void verify(const char *text,long count,const char *state,
                   unsigned reset,unsigned sleep_secs) {
    classic_original_status_decision d=
        classic_original_decide_status(text,count);
    assert(!strcmp(d.state,state));
    assert(d.send_reset_and_end==reset);
    assert(d.sleep_seconds==sleep_secs);
}
int main(void) {
    verify("SKU:Load Error;",15,"com.leitz.out-of-paper-error",1,5);
    verify("USED:-1",7,"com.leitz.out-of-paper-error",1,5);
    verify("SKU:Door Open;",14,"com.leitz.door-open-error",0,5);
    verify("SKU:No Cartridge;",17,"com.leitz.no-cartridge-error",0,5);
    verify("SKU:Bad Cartridge;",18,"com.leitz.general-error",0,0);
    verify("SKU:Ready;",10,"com.leitz.ready",0,0);
    verify("",0,"com.leitz.general-error",0,0);
    verify("",-1,"com.leitz.general-error",0,0);
    puts("original CheckPrinterStatus decision branches passed");
    return 0;
}
