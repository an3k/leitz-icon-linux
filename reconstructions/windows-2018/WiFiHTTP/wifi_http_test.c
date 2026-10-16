#include "wifi_http.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    char out[1024]={0},before[1024]={0};
    size_t n=123;
    assert(leitz_win2018_wifi_scan_path(out,sizeof out,&n)==0);
    assert(n==15 && strcmp(out,"Scan.xml?scan=1")==0);
    assert(leitz_win2018_wifi_scan_row_path(0,out,sizeof out,&n)==0);
    assert(!strcmp(out,"ScanRow.xml?ScanRow=0"));
    assert(leitz_win2018_wifi_scan_row_path(UINT32_MAX,out,sizeof out,&n)==0);
    assert(!strcmp(out,"ScanRow.xml?ScanRow=4294967295"));
    memcpy(before,out,sizeof out);
    assert(leitz_win2018_wifi_scan_row_path(1,out,2,&n)==-1);
    assert(!memcmp(out,before,sizeof out));
    assert(leitz_win2018_wifi_scan_path(NULL,sizeof out,&n)==-1);
    assert(leitz_win2018_wifi_scan_path(out,sizeof out,NULL)==-1);

    const uint8_t name[]="Home",mode[]="WPA",secret[]="abcde";
    assert(leitz_win2018_wifi_infrastructure_form(name,4,mode,3,
        secret,5,out,sizeof out,&n)==0);
    const char original[]=
        "HiddenName=&HiddenSecurity=None&HiddenPassword=&DHCP=DHCP&"
        "WifiSetup=Connect&NetworkType=Infrastructure&Name=Home&"
        "SecurityMode=WPA&SecurityKey=abcde";
    assert(n==strlen(original) && !strcmp(out,original));
    const uint8_t other_name[]="Office & Lab",other_secret[]="a&b=2";
    assert(leitz_win2018_wifi_infrastructure_form(other_name,
        sizeof other_name-1,mode,3,other_secret,
        sizeof other_secret-1,out,sizeof out,&n)==0);
    const char encoded[]=
        "HiddenName=&HiddenSecurity=None&HiddenPassword=&DHCP=DHCP&"
        "WifiSetup=Connect&NetworkType=Infrastructure&"
        "Name=Office%20%26%20Lab&SecurityMode=WPA&SecurityKey=a%26b%3D2";
    assert(n==strlen(encoded) && !strcmp(out,encoded));
    memcpy(before,out,sizeof out);
    assert(leitz_win2018_wifi_infrastructure_form(name,4,mode,3,
        secret,5,out,1,&n)==-1);
    assert(!memcmp(out,before,sizeof out));
    assert(leitz_win2018_wifi_infrastructure_form(NULL,4,mode,3,
        secret,5,out,sizeof out,&n)==-1);
    const uint8_t zero[]={'A',0,'B'};
    assert(leitz_win2018_wifi_infrastructure_form(zero,3,mode,3,
        secret,5,out,sizeof out,&n)==-1);
    leitz_win2018_wifi_clear(out,sizeof out);
    for(size_t i=0;i<sizeof out;++i)assert(out[i]==0);
    puts("Windows AddPrinter 2018 HTTP WiFi scan/form offline reconstruction: PASS");
    return 0;
}
