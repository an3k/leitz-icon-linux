#include "oem_host.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int n_errors;
    uint32_t last_error;
    int queries;
    int releases;
    int32_t query_hr;
    void *query_result;
    void *released;
} harness;
static void error_cb(void *p, uint32_t code)
{
    harness *h = p;
    h->last_error = code;
    ++h->n_errors;
}
static int32_t query_cb(void *p, const uint8_t iid[16], void **out)
{
    static const uint8_t expected[16] = {
        0xf1,0xbb,0x7e,0xd6,0xbf,0x78,0xd1,0x11,
        0x94,0x80,0x00,0xa0,0xc9,0x06,0x40,0xb8
    };
    harness *h = p;
    assert(memcmp(iid, expected, 16) == 0);
    ++h->queries;
    *out = h->query_result;
    return h->query_hr;
}
static void release_cb(void *p, void *interface)
{
    harness *h = p;
    ++h->releases;
    h->released = interface;
}
int main(void)
{
    static const uint8_t expected[16] = {
        0xf1,0xbb,0x7e,0xd6,0xbf,0x78,0xd1,0x11,
        0x94,0x80,0x00,0xa0,0xc9,0x06,0x40,0xb8
    };
    assert(memcmp(leitz_rp_oem_driver_iid(), expected, 16) == 0);
    harness h = {0};
    uint8_t result[8];
    memset(result,0x6a,sizeof result);
    uint32_t needed = 17;
    assert(leitz_rp_oem_get_info(1,result,sizeof result,&needed,error_cb,&h)==0);
    assert(needed==4 && memcmp(result,"DIPL",4)==0 && result[4]==0x6a);
    assert(leitz_rp_oem_get_info(3,result,sizeof result,&needed,error_cb,&h)==0);
    assert(needed==4 && result[0]==1 && result[1]==0 && result[2]==0 && result[3]==0);
    needed=17;
    assert(leitz_rp_oem_get_info(4,result,sizeof result,&needed,error_cb,&h)==LEITZ_RP_E_FAIL);
    assert(needed==8 && h.n_errors==0 && result[4]==0x6a);
    needed=17;
    assert(leitz_rp_oem_get_info(2,result,sizeof result,&needed,error_cb,&h)==LEITZ_RP_E_FAIL);
    assert(needed==17 && h.last_error==87 && h.n_errors==1);
    assert(leitz_rp_oem_get_info(0,result,sizeof result,&needed,error_cb,&h)==LEITZ_RP_E_FAIL);
    assert(h.last_error==87 && h.n_errors==2);
    needed=17;
    assert(leitz_rp_oem_get_info(1,NULL,4,&needed,error_cb,&h)==LEITZ_RP_E_FAIL);
    assert(needed==4 && h.last_error==122 && h.n_errors==3);
    needed=17;
    assert(leitz_rp_oem_get_info(3,result,3,&needed,error_cb,&h)==LEITZ_RP_E_FAIL);
    assert(needed==4 && h.last_error==122 && h.n_errors==4);
    needed=17;
    assert(leitz_rp_oem_get_info(3,result,4,NULL,error_cb,&h)==LEITZ_RP_E_FAIL);
    assert(needed==17 && h.last_error==87 && h.n_errors==5);
    assert(leitz_rp_oem_get_info(4,NULL,0,&needed,NULL,NULL)==LEITZ_RP_E_FAIL);
    assert(needed==8);

    int object1=1, object2=2;
    h.query_result=&object1;
    leitz_rp_oem_binding binding={NULL};
    assert(leitz_rp_oem_publish_driver(&binding,query_cb,&h)==LEITZ_RP_S_OK);
    assert(binding.driver_interface==&object1 && h.queries==1);
    h.query_result=&object2;
    assert(leitz_rp_oem_publish_driver(&binding,query_cb,&h)==LEITZ_RP_S_OK);
    assert(binding.driver_interface==&object1 && h.queries==1);
    assert(leitz_rp_oem_release_driver(&binding,NULL,&h)==LEITZ_RP_E_POINTER);
    assert(binding.driver_interface==&object1 && h.releases==0);
    assert(leitz_rp_oem_release_driver(&binding,release_cb,&h)==LEITZ_RP_S_OK);
    assert(binding.driver_interface==NULL && h.released==&object1 && h.releases==1);
    assert(leitz_rp_oem_release_driver(&binding,release_cb,&h)==LEITZ_RP_S_OK);
    assert(h.releases==1);
    h.query_hr=(int32_t)UINT32_C(0x80004002);
    assert(leitz_rp_oem_publish_driver(&binding,query_cb,&h)==LEITZ_RP_E_FAIL);
    assert(binding.driver_interface==NULL && h.queries==2);
    h.query_hr=(int32_t)UINT32_C(0x8007000e);
    assert(leitz_rp_oem_publish_driver(&binding,query_cb,&h)==LEITZ_RP_E_FAIL);
    assert(binding.driver_interface==NULL && h.queries==3);
    h.query_hr=1;h.query_result=NULL;
    assert(leitz_rp_oem_publish_driver(&binding,query_cb,&h)==LEITZ_RP_S_OK);
    assert(binding.driver_interface==NULL && h.queries==4);
    h.query_hr=0;h.query_result=&object2;
    assert(leitz_rp_oem_publish_driver(&binding,query_cb,&h)==LEITZ_RP_S_OK);
    assert(binding.driver_interface==&object2 && h.queries==5);
    assert(leitz_rp_oem_release_driver(&binding,release_cb,&h)==LEITZ_RP_S_OK);
    assert(h.released==&object2 && h.releases==2);
    assert(leitz_rp_oem_publish_driver(NULL,query_cb,&h)==LEITZ_RP_E_POINTER);
    assert(leitz_rp_oem_publish_driver(&binding,NULL,&h)==LEITZ_RP_E_POINTER);
    assert(leitz_rp_oem_release_driver(NULL,release_cb,&h)==LEITZ_RP_E_POINTER);
    assert(h.queries==5);
    puts("Windows 2018 original LeitzRp GetInfo and OEM driver COM binding: PASS");
    return 0;
}
