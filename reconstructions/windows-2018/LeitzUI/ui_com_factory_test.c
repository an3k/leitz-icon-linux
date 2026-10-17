#include "ui_com_factory.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void check_guid(leitz_win2018_ui_guid_kind k, const uint8_t b[16])
{
    const leitz_win2018_ui_guid *g=leitz_win2018_ui_known_guid(k);
    assert(g != NULL && memcmp(g->bytes,b,16)==0);
}

int main(void)
{
    const uint8_t clsid[16] = {
       0xd7,0x80,0xce,0xab,0x46,0x9f,0xd1,0x11,
       0x88,0x2a,0x00,0xc0,0x4f,0xb9,0x61,0xec};
    const uint8_t uiid[16] = {
       0xd0,0xa9,0xa7,0xc6,0x4c,0x77,0xd1,0x11,
       0x94,0x7f,0x00,0xa0,0xc9,0x06,0x40,0xb8};
    const uint8_t unknown[16] = {
       0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    const uint8_t class_factory[16] = {
       1,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    check_guid(LEITZ_UI_GUID_CLSID,clsid);
    check_guid(LEITZ_UI_GUID_IUNKNOWN,unknown);
    check_guid(LEITZ_UI_GUID_ICLASSFACTORY,class_factory);
    check_guid(LEITZ_UI_GUID_IPRINTOEMUI,uiid);
    assert(leitz_win2018_ui_known_guid((leitz_win2018_ui_guid_kind)-1)==NULL);
    assert(leitz_win2018_ui_known_guid((leitz_win2018_ui_guid_kind)4)==NULL);

    const leitz_win2018_ui_guid *valid=leitz_win2018_ui_known_guid(LEITZ_UI_GUID_CLSID);
    const leitz_win2018_ui_guid *iu=leitz_win2018_ui_known_guid(LEITZ_UI_GUID_IUNKNOWN);
    const leitz_win2018_ui_guid *icf=leitz_win2018_ui_known_guid(LEITZ_UI_GUID_ICLASSFACTORY);
    const leitz_win2018_ui_guid *printer=leitz_win2018_ui_known_guid(LEITZ_UI_GUID_IPRINTOEMUI);
    leitz_win2018_ui_guid random={{0}};
    leitz_win2018_ui_interface out=LEITZ_UI_INTERFACE_PRINTER;
    assert(leitz_win2018_ui_get_class_object(valid,icf,1,&out)==LEITZ_UI_S_OK);
    assert(out==LEITZ_UI_INTERFACE_FACTORY);
    assert(leitz_win2018_ui_get_class_object(valid,iu,1,&out)==LEITZ_UI_S_OK);
    assert(out==LEITZ_UI_INTERFACE_FACTORY);
    assert(leitz_win2018_ui_get_class_object(valid,printer,1,&out)==LEITZ_UI_E_NOINTERFACE);
    assert(out==LEITZ_UI_INTERFACE_NONE);
    assert(leitz_win2018_ui_get_class_object(&random,icf,1,&out)==LEITZ_UI_CLASS_E_CLASSNOTAVAILABLE);
    assert(out==LEITZ_UI_INTERFACE_NONE);
    assert(leitz_win2018_ui_get_class_object(valid,icf,0,&out)==LEITZ_UI_E_OUTOFMEMORY);
    assert(out==LEITZ_UI_INTERFACE_NONE);
    assert(leitz_win2018_ui_get_class_object(valid,icf,1,NULL)==LEITZ_UI_E_POINTER);
    assert(leitz_win2018_ui_get_class_object(NULL,icf,1,&out)==LEITZ_UI_E_POINTER);
    assert(leitz_win2018_ui_get_class_object(valid,NULL,1,&out)==LEITZ_UI_E_POINTER);
    assert(out==LEITZ_UI_INTERFACE_NONE);
    assert(leitz_win2018_ui_query_interface(LEITZ_UI_INTERFACE_FACTORY,iu,&out)==LEITZ_UI_S_OK);
    assert(out==LEITZ_UI_INTERFACE_FACTORY);
    assert(leitz_win2018_ui_query_interface(LEITZ_UI_INTERFACE_FACTORY,printer,&out)==LEITZ_UI_E_NOINTERFACE);
    assert(out==LEITZ_UI_INTERFACE_NONE);
    assert(leitz_win2018_ui_query_interface(LEITZ_UI_INTERFACE_PRINTER,printer,&out)==LEITZ_UI_S_OK);
    assert(out==LEITZ_UI_INTERFACE_PRINTER);
    assert(leitz_win2018_ui_query_interface(LEITZ_UI_INTERFACE_PRINTER,iu,&out)==LEITZ_UI_S_OK);
    assert(leitz_win2018_ui_query_interface(LEITZ_UI_INTERFACE_PRINTER,icf,&out)==LEITZ_UI_E_NOINTERFACE);
    assert(leitz_win2018_ui_query_interface(LEITZ_UI_INTERFACE_NONE,iu,&out)==LEITZ_UI_E_NOINTERFACE);
    assert(leitz_win2018_ui_query_interface(LEITZ_UI_INTERFACE_PRINTER,iu,NULL)==LEITZ_UI_E_POINTER);
    assert(leitz_win2018_ui_query_interface(LEITZ_UI_INTERFACE_PRINTER,NULL,&out)==LEITZ_UI_E_POINTER);
    assert(out==LEITZ_UI_INTERFACE_NONE);

    assert(leitz_win2018_ui_create_instance(1,printer,1,&out)==LEITZ_UI_CLASS_E_NOAGGREGATION);
    assert(out==LEITZ_UI_INTERFACE_NONE);
    assert(leitz_win2018_ui_create_instance(0,printer,0,&out)==LEITZ_UI_E_OUTOFMEMORY);
    assert(out==LEITZ_UI_INTERFACE_NONE);
    assert(leitz_win2018_ui_create_instance(0,printer,1,&out)==LEITZ_UI_S_OK);
    assert(out==LEITZ_UI_INTERFACE_PRINTER);
    assert(leitz_win2018_ui_create_instance(0,iu,1,&out)==LEITZ_UI_S_OK);
    assert(out==LEITZ_UI_INTERFACE_PRINTER);
    assert(leitz_win2018_ui_create_instance(0,icf,1,&out)==LEITZ_UI_E_NOINTERFACE);
    assert(out==LEITZ_UI_INTERFACE_NONE);
    assert(leitz_win2018_ui_create_instance(0,printer,1,NULL)==LEITZ_UI_E_POINTER);
    assert(leitz_win2018_ui_create_instance(0,NULL,1,&out)==LEITZ_UI_E_POINTER);

    atomic_int locks=ATOMIC_VAR_INIT(0), objects=ATOMIC_VAR_INIT(0);
    assert(leitz_win2018_ui_can_unload_now(&objects,&locks)==LEITZ_UI_S_OK);
    assert(leitz_win2018_ui_lock_server(&locks,1)==LEITZ_UI_S_OK);
    assert(leitz_win2018_ui_can_unload_now(&objects,&locks)==LEITZ_UI_S_FALSE);
    assert(leitz_win2018_ui_lock_server(&locks,1)==LEITZ_UI_S_OK);
    assert(atomic_load(&locks)==2);
    assert(leitz_win2018_ui_lock_server(&locks,0)==LEITZ_UI_S_OK);
    assert(leitz_win2018_ui_can_unload_now(&objects,&locks)==LEITZ_UI_S_FALSE);
    assert(leitz_win2018_ui_lock_server(&locks,0)==LEITZ_UI_S_OK);
    assert(atomic_load(&locks)==0);
    atomic_store(&objects,1);
    assert(leitz_win2018_ui_can_unload_now(&objects,&locks)==LEITZ_UI_S_FALSE);
    atomic_store(&objects,0);
    assert(leitz_win2018_ui_can_unload_now(&objects,&locks)==LEITZ_UI_S_OK);
    assert(leitz_win2018_ui_can_unload_now(NULL,&locks)==LEITZ_UI_S_FALSE);
    assert(leitz_win2018_ui_can_unload_now(&objects,NULL)==LEITZ_UI_S_FALSE);
    assert(leitz_win2018_ui_lock_server(NULL,1)==LEITZ_UI_E_POINTER);
    puts("Original x86/x64 LeitzUI COM class factory decision paths: PASS");
    return 0;
}
