#include "oem_lifetime.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>

typedef struct {unsigned calls;void *last;} tracker;
static void release_host(void *p,void *host) {
    tracker *t=p;++t->calls;t->last=host;
}
int main(void) {
    leitz_rp_oem_lifetime raster={0}, factory={0}, second={0};
    atomic_int raster_global=ATOMIC_VAR_INIT(0),lock_server=ATOMIC_VAR_INIT(0);
    atomic_int unload_a=ATOMIC_VAR_INIT(0),unload_b=ATOMIC_VAR_INIT(0);
    tracker t={0};int host=100;unsigned n=99;
    assert(leitz_rp_lifetime_init(NULL,LEITZ_RP_OBJECT_RASTER,&raster_global,&host,release_host,&t)==-1);
    assert(leitz_rp_lifetime_init(&raster,LEITZ_RP_OBJECT_RASTER,NULL,&host,release_host,&t)==-1);
    assert(leitz_rp_lifetime_init(&raster,LEITZ_RP_OBJECT_RASTER,&raster_global,&host,NULL,&t)==-1);
    assert(leitz_rp_lifetime_init(&raster,(leitz_rp_object_kind)3,&raster_global,NULL,NULL,NULL)==-1);
    assert(atomic_load(&raster_global)==0);
    assert(leitz_rp_lifetime_init(&raster,LEITZ_RP_OBJECT_RASTER,&raster_global,&host,release_host,&t)==0);
    assert(atomic_load(&raster_global)==1 && atomic_load(&raster.reference_count)==1);
    assert(atomic_load(&lock_server)==0 && atomic_load(&unload_a)==0 && atomic_load(&unload_b)==0);
    assert(leitz_rp_lifetime_addref(&raster,&n)==0 && n==2);
    assert(leitz_rp_lifetime_addref(&raster,&n)==0 && n==3);
    assert(leitz_rp_lifetime_release(&raster,&n)==0 && n==2 && t.calls==0);
    assert(leitz_rp_lifetime_release(&raster,&n)==0 && n==1 && t.calls==0);
    assert(leitz_rp_lifetime_release(&raster,&n)==0 && n==0);
    assert(t.calls==1 && t.last==&host && raster.cached_host==NULL);
    assert(atomic_load(&raster_global)==0);
    n=77;
    assert(leitz_rp_lifetime_release(&raster,&n)==-1 && n==77);
    assert(leitz_rp_lifetime_addref(&raster,&n)==-1 && n==77);
    assert(t.calls==1 && atomic_load(&raster_global)==0);
    assert(leitz_rp_lifetime_init(&factory,LEITZ_RP_OBJECT_FACTORY,NULL,NULL,NULL,NULL)==0);
    assert(leitz_rp_lifetime_addref(&factory,&n)==0 && n==2);
    assert(leitz_rp_lifetime_release(&factory,&n)==0 && n==1);
    assert(leitz_rp_lifetime_release(&factory,&n)==0 && n==0);
    assert(atomic_load(&raster_global)==0 && t.calls==1);
    assert(leitz_rp_lifetime_init(&second,LEITZ_RP_OBJECT_RASTER,&raster_global,NULL,NULL,NULL)==0);
    assert(atomic_load(&raster_global)==1);
    assert(leitz_rp_lifetime_release(&second,&n)==0 && n==0);
    assert(atomic_load(&raster_global)==0 && t.calls==1);
    atomic_store(&raster_global,INT_MAX);
    assert(leitz_rp_lifetime_init(&second,LEITZ_RP_OBJECT_RASTER,&raster_global,NULL,NULL,NULL)==-1);
    assert(atomic_load(&raster_global)==INT_MAX);
    atomic_store(&raster_global,0);
    assert(leitz_rp_lifetime_init(&second,LEITZ_RP_OBJECT_FACTORY,NULL,NULL,NULL,NULL)==0);
    atomic_store(&second.reference_count,UINT_MAX);
    n=77;
    assert(leitz_rp_lifetime_addref(&second,&n)==-1 && n==77);
    assert(leitz_rp_lifetime_addref(NULL,&n)==-1);
    assert(leitz_rp_lifetime_release(NULL,&n)==-1);
    assert(leitz_rp_lifetime_addref(&second,NULL)==-1);
    assert(leitz_rp_lifetime_release(&second,NULL)==-1);
    puts("LeitzRp original raster/factory AddRef Release and separate tracked global: PASS");
    return 0;
}
