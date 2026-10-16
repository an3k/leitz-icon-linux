#include "wifi_managed.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    assert(!strcmp(leitz_win2018_security_mode(0,0),"None"));
    assert(!strcmp(leitz_win2018_security_mode(1,0),"WEP"));
    assert(!strcmp(leitz_win2018_security_mode(1,5),"WEP40"));
    assert(!strcmp(leitz_win2018_security_mode(1,10),"WEP40"));
    assert(!strcmp(leitz_win2018_security_mode(1,13),"WEP104"));
    assert(!strcmp(leitz_win2018_security_mode(1,26),"WEP104"));
    assert(!strcmp(leitz_win2018_security_mode(1,12),"WEP"));
    for(int i=5;i<=13;i+=4)
        assert(!strcmp(leitz_win2018_security_mode(i,20),"WPA"));
    assert(!strcmp(leitz_win2018_security_mode(4,20),"None"));
    assert(!strcmp(leitz_win2018_security_mode(-1,20),"None"));

    char body[512],snapshot[512];size_t n=123;
    memset(body,0x59,sizeof body);
    assert(leitz_win2018_oem_ascii_form("Net&Guest","WPA","a&b&c",
                                       body,sizeof body,&n)==0);
    const char *expected=
        "HiddenName=&HiddenSecurity=None&HiddenPassword=&DHCP=DHCP&"
        "WifiSetup=Connect&NetworkType=Infrastructure&Name=Net&Guest&"
        "SecurityMode=WPA&SecurityKey=a%26b%26c";
    assert(n==strlen(expected) && !strcmp(body,expected));
    assert(strstr(body,"Name=Net&Guest&SecurityMode=WPA"));
    memcpy(snapshot,body,sizeof body);
    assert(leitz_win2018_oem_ascii_form("Net&Guest","WPA","a&b&c",
                                       body,n,&n)==-1);
    assert(!memcmp(body,snapshot,sizeof body));
    assert(leitz_win2018_oem_ascii_form("\xc3\xa4","WPA","abc",
                                       body,sizeof body,&n)==-1);
    assert(leitz_win2018_oem_ascii_form("abc","WPA",NULL,
                                       body,sizeof body,&n)==-1);
    assert(leitz_win2018_oem_ascii_form("abc","WPA","secret",
                                       NULL,sizeof body,&n)==-1);

    leitz_win2018_scan_progress s={0};
    assert(leitz_win2018_scan_observe(&s,1)==LEITZ_WIFI_SCAN_POLL_NOW);
    for(unsigned i=0;i<50;++i) {
        leitz_win2018_scan_step actual=leitz_win2018_scan_observe(&s,1);
        assert(actual==(i<49?LEITZ_WIFI_SCAN_POLL_AFTER_300MS:
                              LEITZ_WIFI_SCAN_EXHAUSTED));
    }
    assert(s.finished && s.polls==50);
    assert(leitz_win2018_scan_observe(&s,0)==LEITZ_WIFI_SCAN_INVALID);
    leitz_win2018_scan_progress early={0};
    assert(leitz_win2018_scan_observe(&early,0)==LEITZ_WIFI_SCAN_DONE);
    assert(early.finished);
    leitz_win2018_scan_progress second={0};
    assert(leitz_win2018_scan_observe(&second,1)==LEITZ_WIFI_SCAN_POLL_NOW);
    assert(leitz_win2018_scan_observe(&second,0)==LEITZ_WIFI_SCAN_DONE);
    assert(second.polls==1);
    assert(leitz_win2018_scan_observe(NULL,0)==LEITZ_WIFI_SCAN_INVALID);
    puts("Windows 2018 AddPrinter OEM security/form/scan policy: PASS");
    return 0;
}
