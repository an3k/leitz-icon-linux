#include "wifi_http.h"
#include <stdint.h>
#include <string.h>

static int constant(const char *s,char *out,size_t cap,size_t *length)
{
    if(!s||!out||!length)return -1;
    size_t n=strlen(s);
    if(n==SIZE_MAX || n+1>cap)return -1;
    memcpy(out,s,n+1);
    *length=n;
    return 0;
}
int leitz_win2018_wifi_scan_path(char *out,size_t cap,size_t *length)
{
    return constant("Scan.xml?scan=1",out,cap,length);
}
int leitz_win2018_wifi_scan_row_path(uint32_t row,char *out,
                                     size_t capacity,size_t *length)
{
    static const char prefix[]="ScanRow.xml?ScanRow=";
    if(!out||!length)return -1;
    char reversed[10];
    size_t used=0;
    do {
        reversed[used++]=(char)('0'+row%10u);
        row/=10u;
    } while(row);
    size_t n=sizeof prefix-1;
    if(n>SIZE_MAX-used-1 || n+used+1>capacity)return -1;
    memcpy(out,prefix,n);
    for(size_t i=0;i<used;++i)out[n+i]=reversed[used-1-i];
    out[n+used]=0;
    *length=n+used;
    return 0;
}
static int unreserved(uint8_t c)
{
    return (c>='A'&&c<='Z')||(c>='a'&&c<='z')||
           (c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~';
}
static size_t escaped_length(const uint8_t *data,size_t n)
{
    size_t size=0;
    for(size_t i=0;i<n;++i) {
        size_t delta=unreserved(data[i])?1:3;
        if(size>SIZE_MAX-delta)return SIZE_MAX;
        size+=delta;
    }
    return size;
}
static size_t escape_into(char *out,const uint8_t *src,size_t length)
{
    static const char digits[]="0123456789ABCDEF";
    size_t count=0;
    for(size_t i=0;i<length;++i) {
        if(unreserved(src[i]))out[count++]=(char)src[i];
        else {
            out[count++]='%';
            out[count++]=digits[src[i]>>4];
            out[count++]=digits[src[i]&15];
        }
    }
    return count;
}
int leitz_win2018_wifi_infrastructure_form(
    const uint8_t *ssid,size_t ssid_len,
    const uint8_t *mode,size_t mode_len,
    const uint8_t *secret,size_t secret_len,
    char *out,size_t capacity,size_t *length)
{
    static const char start[]=
        "HiddenName=&HiddenSecurity=None&HiddenPassword=&DHCP=DHCP&"
        "WifiSetup=Connect&NetworkType=Infrastructure&Name=";
    static const char sec[]="&SecurityMode=";
    static const char key[]="&SecurityKey=";
    if(!out||!length||(ssid_len&&!ssid)||(mode_len&&!mode)||
       (secret_len&&!secret))return -1;
    /* Embedded NULs are forbidden in managed form fields. */
    for(size_t i=0;i<ssid_len;++i)if(!ssid[i])return -1;
    for(size_t i=0;i<mode_len;++i)if(!mode[i])return -1;
    for(size_t i=0;i<secret_len;++i)if(!secret[i])return -1;
    size_t n1=escaped_length(ssid,ssid_len);
    size_t n2=escaped_length(mode,mode_len);
    size_t n3=escaped_length(secret,secret_len);
    const size_t fixed=(sizeof start-1)+(sizeof sec-1)+(sizeof key-1);
    if(n1==SIZE_MAX||n2==SIZE_MAX||n3==SIZE_MAX ||
       n1>SIZE_MAX-fixed || n2>SIZE_MAX-fixed-n1 ||
       n3>=SIZE_MAX-fixed-n1-n2)return -1;
    size_t total=fixed+n1+n2+n3;
    if(total+1>capacity)return -1;
    size_t p=0;
    memcpy(out+p,start,sizeof start-1);p+=sizeof start-1;
    p+=escape_into(out+p,ssid,ssid_len);
    memcpy(out+p,sec,sizeof sec-1);p+=sizeof sec-1;
    p+=escape_into(out+p,mode,mode_len);
    memcpy(out+p,key,sizeof key-1);p+=sizeof key-1;
    p+=escape_into(out+p,secret,secret_len);
    out[p]=0;*length=p;
    return 0;
}
void leitz_win2018_wifi_clear(void *buffer,size_t size)
{
    if(!buffer)return;
    volatile uint8_t *p=buffer;
    while(size--)*p++=0;
}
