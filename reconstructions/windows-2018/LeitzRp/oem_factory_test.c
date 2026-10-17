#include "oem_factory.h"
#include "oem_host.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void matches(leitz_rp_guid_kind kind,const uint8_t bytes[16])
{
    const leitz_rp_guid *got=leitz_rp_known_guid(kind);
    assert(got && memcmp(got->bytes,bytes,16)==0);
}
int main(void)
{
    const uint8_t classid[16]={0x26,0xbf,0x6a,0x6d,0x38,0x9f,0xd1,0x11,
       0x88,0x2a,0x00,0xc0,0x4f,0xb9,0x61,0xec};
    const uint8_t iunk[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    const uint8_t iclass[16]={1,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    const uint8_t iraster[16]={0xf0,0xbb,0x7e,0xd6,0xbf,0x78,0xd1,0x11,
       0x94,0x80,0,0xa0,0xc9,0x06,0x40,0xb8};
    matches(LEITZ_RP_GUID_CLSID,classid);
    matches(LEITZ_RP_GUID_IUNKNOWN,iunk);
    matches(LEITZ_RP_GUID_ICLASSFACTORY,iclass);
    matches(LEITZ_RP_GUID_IPRINTOEMUNI,iraster);
    assert(!leitz_rp_known_guid((leitz_rp_guid_kind)-1));
    assert(!leitz_rp_known_guid((leitz_rp_guid_kind)4));
    /* The plugin's OEM raster COM IID is distinct from the host-provided
       OEM driver interface IID it requests from Unidrv. */
    assert(memcmp(iraster,leitz_rp_oem_driver_iid(),16)!=0);
    assert(iraster[0]==0xf0 && leitz_rp_oem_driver_iid()[0]==0xf1);

    const leitz_rp_guid *cls=leitz_rp_known_guid(LEITZ_RP_GUID_CLSID),
      *unk=leitz_rp_known_guid(LEITZ_RP_GUID_IUNKNOWN),
      *factory=leitz_rp_known_guid(LEITZ_RP_GUID_ICLASSFACTORY),
      *raster=leitz_rp_known_guid(LEITZ_RP_GUID_IPRINTOEMUNI);
    leitz_rp_guid random={{0}};
    leitz_rp_interface o=LEITZ_RP_INTERFACE_RASTER;
    assert(leitz_rp_get_class_object(cls,factory,1,&o)==LEITZ_RP_FACTORY_S_OK);
    assert(o==LEITZ_RP_INTERFACE_FACTORY);
    assert(leitz_rp_get_class_object(cls,unk,1,&o)==LEITZ_RP_FACTORY_S_OK);
    assert(o==LEITZ_RP_INTERFACE_FACTORY);
    assert(leitz_rp_get_class_object(cls,raster,1,&o)==LEITZ_RP_FACTORY_E_NOINTERFACE);
    assert(o==LEITZ_RP_INTERFACE_NONE);
    assert(leitz_rp_get_class_object(&random,factory,1,&o)==LEITZ_RP_FACTORY_E_CLASSNOTAVAILABLE);
    assert(o==LEITZ_RP_INTERFACE_NONE);
    assert(leitz_rp_get_class_object(cls,factory,0,&o)==LEITZ_RP_FACTORY_E_OUTOFMEMORY);
    assert(o==LEITZ_RP_INTERFACE_NONE);
    assert(leitz_rp_get_class_object(cls,NULL,1,&o)==LEITZ_RP_FACTORY_E_POINTER);
    assert(leitz_rp_get_class_object(NULL,factory,1,&o)==LEITZ_RP_FACTORY_E_POINTER);
    assert(leitz_rp_get_class_object(cls,factory,1,NULL)==LEITZ_RP_FACTORY_E_POINTER);
    assert(leitz_rp_query_interface(LEITZ_RP_INTERFACE_FACTORY,unk,&o)==LEITZ_RP_FACTORY_S_OK);
    assert(o==LEITZ_RP_INTERFACE_FACTORY);
    assert(leitz_rp_query_interface(LEITZ_RP_INTERFACE_FACTORY,raster,&o)==LEITZ_RP_FACTORY_E_NOINTERFACE);
    assert(o==LEITZ_RP_INTERFACE_NONE);
    assert(leitz_rp_query_interface(LEITZ_RP_INTERFACE_RASTER,raster,&o)==LEITZ_RP_FACTORY_S_OK);
    assert(o==LEITZ_RP_INTERFACE_RASTER);
    assert(leitz_rp_query_interface(LEITZ_RP_INTERFACE_RASTER,unk,&o)==LEITZ_RP_FACTORY_S_OK);
    assert(leitz_rp_query_interface(LEITZ_RP_INTERFACE_RASTER,factory,&o)==LEITZ_RP_FACTORY_E_NOINTERFACE);
    assert(leitz_rp_query_interface(LEITZ_RP_INTERFACE_NONE,unk,&o)==LEITZ_RP_FACTORY_E_NOINTERFACE);
    assert(leitz_rp_query_interface(LEITZ_RP_INTERFACE_FACTORY,NULL,&o)==LEITZ_RP_FACTORY_E_POINTER);
    assert(o==LEITZ_RP_INTERFACE_NONE);
    assert(leitz_rp_query_interface(LEITZ_RP_INTERFACE_FACTORY,unk,NULL)==LEITZ_RP_FACTORY_E_POINTER);
    assert(leitz_rp_create_instance(1,raster,1,&o)==LEITZ_RP_FACTORY_E_NOAGGREGATION);
    assert(o==LEITZ_RP_INTERFACE_NONE);
    assert(leitz_rp_create_instance(0,raster,0,&o)==LEITZ_RP_FACTORY_E_OUTOFMEMORY);
    assert(leitz_rp_create_instance(0,raster,1,&o)==LEITZ_RP_FACTORY_S_OK);
    assert(o==LEITZ_RP_INTERFACE_RASTER);
    assert(leitz_rp_create_instance(0,unk,1,&o)==LEITZ_RP_FACTORY_S_OK);
    assert(o==LEITZ_RP_INTERFACE_RASTER);
    assert(leitz_rp_create_instance(0,factory,1,&o)==LEITZ_RP_FACTORY_E_NOINTERFACE);
    assert(o==LEITZ_RP_INTERFACE_NONE);
    assert(leitz_rp_create_instance(0,NULL,1,&o)==LEITZ_RP_FACTORY_E_POINTER);
    assert(leitz_rp_create_instance(0,unk,1,NULL)==LEITZ_RP_FACTORY_E_POINTER);

    atomic_int locks=ATOMIC_VAR_INIT(0);
    atomic_int unload_global_a=ATOMIC_VAR_INIT(0);
    atomic_int unload_global_b=ATOMIC_VAR_INIT(0);
    assert(leitz_rp_can_unload_now(&unload_global_a,&unload_global_b)==LEITZ_RP_FACTORY_S_OK);
    assert(leitz_rp_lock_server(&locks,1)==LEITZ_RP_FACTORY_S_OK);
    assert(atomic_load(&locks)==1);
    /* Native DllCanUnloadNow reads 0x76ec/0x76e8 (x64), not the
       LockServer counter 0x76e4. Deliberately preserve separation. */
    assert(leitz_rp_can_unload_now(&unload_global_a,&unload_global_b)==LEITZ_RP_FACTORY_S_OK);
    atomic_store(&unload_global_a,1);
    assert(leitz_rp_can_unload_now(&unload_global_a,&unload_global_b)==LEITZ_RP_FACTORY_S_FALSE);
    atomic_store(&unload_global_a,0);
    atomic_store(&unload_global_b,1);
    assert(leitz_rp_can_unload_now(&unload_global_a,&unload_global_b)==LEITZ_RP_FACTORY_S_FALSE);
    atomic_store(&unload_global_b,0);
    assert(leitz_rp_lock_server(&locks,0)==LEITZ_RP_FACTORY_S_OK);
    assert(atomic_load(&locks)==0);
    assert(leitz_rp_can_unload_now(NULL,&unload_global_b)==LEITZ_RP_FACTORY_S_FALSE);
    assert(leitz_rp_lock_server(NULL,1)==LEITZ_RP_FACTORY_E_POINTER);
    puts("Windows LeitzRp original COM factory, raster IID and unload counters: PASS");
    return 0;
}
