#include "sendrecv_result.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const uint8_t original[]={0x1b,'V',0x00,0xa5,0x7f};
    uint8_t dst[16];
    size_t n=0x12345678u;
    memset(dst,0xcc,sizeof dst);
    assert(leitz_win2018_sendrecv_copy_result(0,0,0,original,sizeof original,
                                               dst,sizeof dst,&n)==1);
    assert(n==sizeof original && !memcmp(dst,original,n));
    assert(dst[n]==0xcc);

    memset(dst,0xcc,sizeof dst);n=21;
    assert(leitz_win2018_sendrecv_copy_result(0,0,0,original,0,
                                               dst,sizeof dst,&n)==0);
    assert(n==21 && dst[0]==0xcc);
    assert(leitz_win2018_sendrecv_copy_result((int32_t)0x80004005u,0,0,
                        original,sizeof original,dst,sizeof dst,&n)==0);
    assert(n==21 && dst[0]==0xcc);
    assert(leitz_win2018_sendrecv_copy_result(1,0,0,
                        original,sizeof original,dst,sizeof dst,&n)==0);
    assert(n==21 && dst[0]==0xcc);
    assert(leitz_win2018_sendrecv_copy_result(0,1,0,original,
                        sizeof original,dst,sizeof dst,&n)==0);
    assert(leitz_win2018_sendrecv_copy_result(0,0,1,original,
                        sizeof original,dst,sizeof dst,&n)==0);
    assert(n==21 && dst[0]==0xcc);

    assert(leitz_win2018_sendrecv_copy_result(0,0,0,original,
                        sizeof original,dst,4,&n)==-1);
    assert(n==21 && dst[0]==0xcc);
    assert(leitz_win2018_sendrecv_copy_result(0,0,0,NULL,
                        sizeof original,dst,sizeof dst,&n)==-1);
    assert(leitz_win2018_sendrecv_copy_result(0,0,0,original,
                        sizeof original,NULL,sizeof dst,&n)==-1);
    assert(leitz_win2018_sendrecv_copy_result(0,0,0,original,
                        sizeof original,dst,sizeof dst,NULL)==-1);
    assert(n==21 && dst[0]==0xcc);

    memcpy(dst,original,sizeof original);
    assert(leitz_win2018_sendrecv_copy_result(0,0,0,dst,
                        sizeof original,dst,sizeof dst,&n)==1);
    assert(n==sizeof original && !memcmp(dst,original,n));
    puts("Windows PrintHelper SendRecvData bounded result transfer: PASS");
    return 0;
}
