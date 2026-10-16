#include "status_probe.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    /* Windows PRINTER_INFO_2.Attributes 0x400 is WORK_OFFLINE,
     * not the hardware status DWORD at offset +0x48. */
    assert(leitz_win2018_is_queue_online(1,1,0)==1);
    assert(leitz_win2018_is_queue_online(1,1,0x400)==0);
    assert(leitz_win2018_is_queue_online(1,1,0x401)==0);
    assert(leitz_win2018_is_queue_online(1,1,0x80)==1);
    assert(leitz_win2018_is_queue_online(1,0,0)==0);
    assert(leitz_win2018_is_queue_online(0,1,0)==0);

    int boot=0;
    static const uint8_t version[]="Leitz Icon 2.17";
    assert(leitz_win2018_classify_version_probe(1,1,version,
        sizeof version,&boot)==1 && boot==0);
    static const uint8_t boot_text[]="FW:BOOT-123";
    assert(leitz_win2018_classify_version_probe(1,1,boot_text,
        sizeof boot_text,&boot)==1 && boot==1);
    static const uint8_t small[]="BOOT";
    assert(leitz_win2018_classify_version_probe(1,1,small,4,&boot)==1 && boot==1);
    assert(leitz_win2018_classify_version_probe(0,1,small,4,&boot)==0 && boot==1);
    assert(leitz_win2018_classify_version_probe(1,0,small,4,&boot)==0 && boot==1);

    const uint8_t lowercase[]="boot";
    boot=0;
    assert(leitz_win2018_classify_version_probe(1,1,lowercase,
        sizeof lowercase,&boot)==1 && boot==0);
    const uint8_t embedded[]={'X',0,'B','O','O','T'};
    assert(leitz_win2018_classify_version_probe(1,1,embedded,
        sizeof embedded,&boot)==1 && boot==0);
    assert(leitz_win2018_classify_version_probe(1,1,small,4,&boot)==1 && boot==1);
    assert(leitz_win2018_classify_version_probe(1,1,NULL,0,&boot)==1 && boot==1);
    assert(leitz_win2018_classify_version_probe(1,1,NULL,1,&boot)==-1);
    assert(leitz_win2018_classify_version_probe(1,1,small,65,&boot)==-1);
    assert(leitz_win2018_classify_version_probe(1,1,small,4,NULL)==-1);
    puts("Windows PrintHelper queue/work-offline and ESC V version probe: PASS");
}
