#include "wifi_managed.h"
#include <string.h>

const char *leitz_win2018_security_mode(int32_t mode,size_t key_length)
{
    /* OEM managed GetSecurityModeString RVA 0x6a88:
     * mode 0/other: None; mode 1: WEP (key 5/10 -> WEP40,
     * key 13/26 -> WEP104); modes 5,9,13: WPA.
     */
    if(mode==1) {
        if(key_length==5 || key_length==10)return "WEP40";
        if(key_length==13 || key_length==26)return "WEP104";
        return "WEP";
    }
    if(mode==5 || mode==9 || mode==13)return "WPA";
    return "None";
}
static int bounded_ascii(const char *s,size_t *n)
{
    if(!s)return -1;
    size_t i=0;
    while(i<4096 && s[i]) {
        if((unsigned char)s[i]>127)return -1;
        ++i;
    }
    if(i==4096)return -1; /* new safe cap, not an OEM limit */
    *n=i;
    return 0;
}
int leitz_win2018_oem_ascii_form(const char *ssid,const char *mode,
                                 const char *key,char *out,
                                 size_t capacity,size_t *written)
{
    static const char a[]=
        "HiddenName=&HiddenSecurity=None&HiddenPassword=&DHCP=DHCP&"
        "WifiSetup=Connect&NetworkType=Infrastructure&Name=";
    static const char b[]="&SecurityMode=";
    static const char c[]="&SecurityKey=";
    size_t n1=0,n2=0,n3=0,amp=0;
    if(!out||!written||bounded_ascii(ssid,&n1)||
       bounded_ascii(mode,&n2)||bounded_ascii(key,&n3))return -1;
    for(size_t i=0;i<n3;++i)if(key[i]=='&')++amp;
    const size_t fixed=sizeof a+sizeof b+sizeof c-3;
    if(n1>SIZE_MAX-fixed||n2>SIZE_MAX-fixed-n1||
       n3>SIZE_MAX-fixed-n1-n2||
       amp>(SIZE_MAX-fixed-n1-n2-n3)/2)return -1;
    const size_t total=fixed+n1+n2+n3+2*amp;
    if(total==SIZE_MAX||total+1>capacity)return -1;
    size_t p=0;
    memcpy(out+p,a,sizeof a-1);p+=sizeof a-1;
    memcpy(out+p,ssid,n1);p+=n1;
    memcpy(out+p,b,sizeof b-1);p+=sizeof b-1;
    memcpy(out+p,mode,n2);p+=n2;
    memcpy(out+p,c,sizeof c-1);p+=sizeof c-1;
    for(size_t i=0;i<n3;++i) {
        if(key[i]=='&'){memcpy(out+p,"%26",3);p+=3;}
        else out[p++]=key[i];
    }
    out[p]=0;*written=p;
    return 0;
}
leitz_win2018_scan_step leitz_win2018_scan_observe(
    leitz_win2018_scan_progress *s,int in_progress)
{
    if(!s||s->finished||s->polls>50||
       (in_progress!=0 && in_progress!=1))return LEITZ_WIFI_SCAN_INVALID;
    if(!s->initial_observed) {
        s->initial_observed=1;
        if(!in_progress){s->finished=1;return LEITZ_WIFI_SCAN_DONE;}
        return LEITZ_WIFI_SCAN_POLL_NOW;
    }
    ++s->polls;
    if(!in_progress){s->finished=1;return LEITZ_WIFI_SCAN_DONE;}
    if(s->polls>=50){s->finished=1;return LEITZ_WIFI_SCAN_EXHAUSTED;}
    return LEITZ_WIFI_SCAN_POLL_AFTER_300MS;
}
