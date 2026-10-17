#include "leitz_ui_recovered.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {unsigned calls; uint32_t last;} err_ctx;
static void callback(void *ctx, uint32_t code) {
    err_ctx *s=ctx; ++s->calls; s->last=code;
}
int main(void)
{
    err_ctx e={0};
    uint8_t bytes[8]; memset(bytes,0xa5,sizeof bytes);
    uint32_t needed=91;
    assert(leitz_win2018_ui_get_info(1u, bytes, sizeof bytes, &needed, callback, &e)==0);
    assert(needed==4 && !memcmp(bytes,"DIPL",4) && bytes[4]==0xa5);
    assert(leitz_win2018_ui_get_info(3u, bytes, sizeof bytes, &needed, callback, &e)==0);
    assert(bytes[0]==1 && bytes[1]==0 && bytes[2]==0 && bytes[3]==0 && bytes[4]==0xa5);
    assert(leitz_win2018_ui_get_info(4u, bytes, sizeof bytes, &needed, callback, &e)==LEITZ_UI_E_FAIL);
    assert(needed==0 && e.last==50 && e.calls==1 && bytes[0]==1);
    needed=91;
    assert(leitz_win2018_ui_get_info(2u, bytes, sizeof bytes, &needed, callback, &e)==LEITZ_UI_E_FAIL);
    assert(needed==91 && e.last==87 && e.calls==2);
    assert(leitz_win2018_ui_get_info(5u, bytes, sizeof bytes, &needed, callback, &e)==LEITZ_UI_E_FAIL);
    assert(needed==91 && e.last==87 && e.calls==3);
    assert(leitz_win2018_ui_get_info(1u, NULL, 4u, &needed, callback, &e)==LEITZ_UI_E_FAIL);
    assert(needed==4 && e.last==122 && e.calls==4);
    needed=91;
    assert(leitz_win2018_ui_get_info(3u, bytes, 3u, &needed, callback, &e)==LEITZ_UI_E_FAIL);
    assert(needed==4 && e.last==122 && e.calls==5);
    needed=91;
    assert(leitz_win2018_ui_get_info(1u,bytes,8u,NULL,callback,&e)==LEITZ_UI_E_FAIL);
    assert(needed==91 && e.last==87 && e.calls==6);
    leitz_win2018_ui_cut_state cut={.ui_selected=7,.oem_cut_after_each_label=1,.oem_flags=0x100};
    assert(leitz_win2018_printer_ui_cut_callback(5,&cut)==0 && cut.ui_selected==7);
    assert(leitz_win2018_printer_ui_cut_callback(6,&cut)==0 && cut.ui_selected==1);
    assert(cut.oem_flags==0x100);
    cut.ui_selected=4;
    assert(leitz_win2018_document_ui_cut_callback(10,&cut)==1);
    assert(cut.oem_cut_after_each_label==4 && cut.oem_flags==0x108);
    assert(leitz_win2018_document_ui_cut_callback(10,&cut)==0);
    cut.ui_selected=8;
    assert(leitz_win2018_document_ui_cut_callback(11,&cut)==0 && cut.ui_selected==4);
    cut.ui_selected=9;
    assert(leitz_win2018_document_ui_cut_callback(6,&cut)==0 && cut.ui_selected==4);
    assert(leitz_win2018_document_ui_cut_callback(7,&cut)==0);
    cut.oem_cut_after_each_label=0x10001;
    cut.ui_selected=1;
    assert(leitz_win2018_document_ui_cut_callback(10,&cut)==0);
    assert(cut.oem_cut_after_each_label==0x10001);
    cut.ui_selected=2;
    assert(leitz_win2018_document_ui_cut_callback(10,&cut)==1);
    assert(cut.oem_cut_after_each_label==2);
    assert(leitz_win2018_document_ui_cut_callback(6,NULL)==0);
    assert(leitz_win2018_printer_ui_cut_callback(6,NULL)==0);
    puts("LeitzUI 2018 OEM GetInfo + CutAfterEachLabel callbacks: PASS");
    return 0;
}
