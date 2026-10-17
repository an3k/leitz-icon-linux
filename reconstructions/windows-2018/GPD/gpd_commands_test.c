#include "gpd_commands.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void check(leitz_win2018_gpd_command command,
                  const uint8_t *expected, size_t length)
{
    uint8_t buf[130];
    size_t n = 12345;
    memset(buf, 0xa5, sizeof buf);
    assert(leitz_win2018_gpd_command_length(command) == length);
    assert(leitz_win2018_gpd_copy_command(command, buf, length, &n) == 0);
    assert(n == length);
    assert(!memcmp(buf, expected, length));
    assert(buf[length] == 0xa5);
    memset(buf, 0xa5, sizeof buf);
    n = 12345;
    assert(leitz_win2018_gpd_copy_command(command,buf,length-1,&n) == -1);
    assert(n == 12345);
    for (size_t i=0; i<sizeof buf; i++) assert(buf[i] == 0xa5);
}

int main(void)
{
    uint8_t start_doc[122];
    memset(start_doc, 0x1b, 121);
    start_doc[121] = '@';
    const uint8_t start_page[] = {0x1b,'B',0};
    const uint8_t end_job[] = {0x1b,'E',0x1b,'@',0x1b,'Z'};
    const uint8_t dhl[] = {0x1b,'L',0x08,0x36,0x1b,'w',0x03,0xc0};
    check(LEITZ_WIN2018_GPD_START_DOCUMENT,start_doc,sizeof start_doc);
    check(LEITZ_WIN2018_GPD_START_PAGE,start_page,sizeof start_page);
    check(LEITZ_WIN2018_GPD_END_JOB,end_job,sizeof end_job);
    check(LEITZ_WIN2018_GPD_DHL_SHIPPING_LABEL,dhl,sizeof dhl);
    assert(leitz_win2018_gpd_command_length((leitz_win2018_gpd_command)-1)==0);
    assert(leitz_win2018_gpd_command_length((leitz_win2018_gpd_command)4)==0);
    uint8_t out[130];
    memset(out,0x55,sizeof out);
    size_t written=789;
    assert(leitz_win2018_gpd_copy_command((leitz_win2018_gpd_command)99,
                   out,sizeof out,&written)==-1);
    assert(leitz_win2018_gpd_copy_command(LEITZ_WIN2018_GPD_END_JOB,
                   NULL,sizeof out,&written)==-1);
    assert(leitz_win2018_gpd_copy_command(LEITZ_WIN2018_GPD_END_JOB,
                   out,sizeof out,NULL)==-1);
    assert(written==789 && out[0]==0x55);
    puts("Windows LeitzIcon.gpd fixed byte commands: PASS");
    return 0;
}
