#include "ui_driver_interface.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned queries, releases;
    int32_t query_hresult;
    void *offered;
    const void *released;
} trace;

static int32_t query(void *p, const uint8_t iid[16], void **output)
{
    trace *t = p;
    const uint8_t original_iid[16] = {
        0x50,0x5d,0xb0,0x92,0xbc,0x78,0xd1,0x11,
        0x94,0x80,0x00,0xa0,0xc9,0x06,0x40,0xb8
    };
    assert(!memcmp(iid, original_iid, sizeof original_iid));
    ++t->queries;
    *output = t->offered;
    return t->query_hresult;
}
static void release(void *p, void *interface)
{
    trace *t = p;
    ++t->releases;
    t->released = interface;
}
int main(void)
{
    const uint8_t expected[16] = {
        0x50,0x5d,0xb0,0x92,0xbc,0x78,0xd1,0x11,
        0x94,0x80,0x00,0xa0,0xc9,0x06,0x40,0xb8
    };
    assert(!memcmp(leitz_win2018_ui_driver_iid(), expected, 16));
    int specimen = 123, another = 456;
    leitz_win2018_ui_driver_binding binding = {NULL};
    trace t = {.offered = &specimen};
    assert(leitz_win2018_ui_publish_driver_interface(&binding, &t, query)
           == LEITZ_WIN2018_UI_DRIVER_S_OK);
    assert(binding.driver_interface == &specimen && t.queries == 1);
    /* Native returns immediately when the saved pointer is non-NULL. */
    t.offered = &another;
    assert(leitz_win2018_ui_publish_driver_interface(&binding, &t, query)
           == LEITZ_WIN2018_UI_DRIVER_S_OK);
    assert(binding.driver_interface == &specimen && t.queries == 1);
    assert(leitz_win2018_ui_publish_driver_interface(&binding, &t, NULL)
           == LEITZ_WIN2018_UI_DRIVER_S_OK);
    assert(t.queries == 1);
    assert(leitz_win2018_ui_unbind_driver_interface(&binding, &t, NULL)
           == LEITZ_WIN2018_UI_DRIVER_E_POINTER);
    assert(binding.driver_interface == &specimen);
    assert(leitz_win2018_ui_unbind_driver_interface(&binding, &t, release)
           == LEITZ_WIN2018_UI_DRIVER_S_OK);
    assert(binding.driver_interface == NULL && t.releases == 1);
    assert(t.released == &specimen);
    assert(leitz_win2018_ui_unbind_driver_interface(&binding, &t, release)
           == LEITZ_WIN2018_UI_DRIVER_S_OK && t.releases == 1);
    /* OEM collapses any negative QueryInterface result to E_FAIL. */
    t.offered = &another;
    t.query_hresult = (int32_t)UINT32_C(0x80004002);
    assert(leitz_win2018_ui_publish_driver_interface(&binding, &t, query)
           == LEITZ_WIN2018_UI_DRIVER_E_FAIL);
    assert(t.queries == 2 && binding.driver_interface == NULL);
    t.query_hresult = (int32_t)UINT32_C(0x8007000e);
    assert(leitz_win2018_ui_publish_driver_interface(&binding, &t, query)
           == LEITZ_WIN2018_UI_DRIVER_E_FAIL);
    assert(t.queries == 3 && binding.driver_interface == NULL);
    /* A non-negative HRESULT succeeds even if the native output is NULL. */
    t.query_hresult = 1;
    t.offered = NULL;
    assert(leitz_win2018_ui_publish_driver_interface(&binding, &t, query)
           == LEITZ_WIN2018_UI_DRIVER_S_OK);
    assert(t.queries == 4 && binding.driver_interface == NULL);
    t.query_hresult = 0;
    t.offered = &another;
    assert(leitz_win2018_ui_publish_driver_interface(&binding, &t, query)
           == LEITZ_WIN2018_UI_DRIVER_S_OK);
    assert(t.queries == 5 && binding.driver_interface == &another);
    assert(leitz_win2018_ui_unbind_driver_interface(&binding, &t, release)
           == LEITZ_WIN2018_UI_DRIVER_S_OK);
    assert(t.releases == 2 && t.released == &another);
    assert(leitz_win2018_ui_publish_driver_interface(NULL, &t, query)
           == LEITZ_WIN2018_UI_DRIVER_E_POINTER);
    assert(leitz_win2018_ui_publish_driver_interface(&binding, &t, NULL)
           == LEITZ_WIN2018_UI_DRIVER_E_POINTER);
    assert(leitz_win2018_ui_unbind_driver_interface(NULL, &t, release)
           == LEITZ_WIN2018_UI_DRIVER_E_POINTER);
    assert(t.queries == 5);
    puts("LeitzUI original driver QueryInterface and release: PASS");
    return 0;
}
