#include "wifi_auth.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    assert(LEITZ_WIN2018_PANEL_HTTP_TIMEOUT_MS==15000u);
    assert(LEITZ_WIN2018_WIFI_SCAN_HTTP_TIMEOUT_MS==18000u);
    const uint16_t ssid[]={'i','c','o','n','-','1','2','3'};
    const uint16_t caps[]={'I','C','O','N','-','1','2','3'};
    const uint16_t other[]={'i','c','o','n'};
    assert(leitz_win2018_same_network_name(ssid,8,ssid,8)==1);
    assert(leitz_win2018_same_network_name(ssid,8,caps,8)==0);
    assert(leitz_win2018_same_network_name(ssid,8,other,4)==0);
    assert(leitz_win2018_same_network_name(NULL,0,ssid,8)==0);
    assert(leitz_win2018_same_network_name(NULL,0,NULL,0)==0);
    assert(leitz_win2018_same_network_name(NULL,3,ssid,8)==-1);
    assert(leitz_win2018_same_network_name(ssid,8,NULL,8)==-1);
    const uint16_t embedded[]={'a',0,'b'};
    assert(leitz_win2018_same_network_name(embedded,3,embedded,3)==1);

    assert(leitz_win2018_panel_http_status(LEITZ_WIN2018_HTTP_REPLY,0,200)==3);
    assert(leitz_win2018_panel_http_status(LEITZ_WIN2018_HTTP_REPLY,0,204)==3);
    assert(leitz_win2018_panel_http_status(LEITZ_WIN2018_WEB_EXCEPTION,7,401)==1);
    assert(leitz_win2018_panel_http_status(LEITZ_WIN2018_WEB_EXCEPTION,7,403)==2);
    assert(leitz_win2018_panel_http_status(LEITZ_WIN2018_WEB_EXCEPTION,1,401)==2);
    assert(leitz_win2018_panel_http_status(LEITZ_WIN2018_WEB_EXCEPTION,14,0)==2);
    assert(leitz_win2018_panel_http_status(LEITZ_WIN2018_OTHER_EXCEPTION,7,401)==2);
    assert(leitz_win2018_panel_http_status((leitz_win2018_http_outcome)99,7,401)==-1);

    assert(leitz_win2018_check_login_status(0,3)==2);
    assert(leitz_win2018_check_login_status(1,3)==3);
    assert(leitz_win2018_check_login_status(1,1)==1);
    assert(leitz_win2018_check_login_status(1,2)==2);
    for(int status=-4;status<=10;++status) {
        assert(leitz_win2018_process_credentials_status(status,1)==status);
        assert(leitz_win2018_process_credentials_status(status,0)==
               (status==3?3:4));
    }
    assert(leitz_win2018_process_credentials_status(
           leitz_win2018_check_login_status(0,0),0)==4);
    assert(leitz_win2018_process_credentials_status(
           leitz_win2018_check_login_status(0,0),1)==2);
    puts("Windows 2018 original AddPrinter/LeitzIconCommon HTTP auth decisions: PASS");
    return 0;
}
