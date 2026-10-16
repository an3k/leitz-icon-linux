#include "wifi_password.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void ascii(uint16_t *out,size_t cap,const char *input)
{
    size_t n=strlen(input);assert(n+1<=cap);
    for(size_t i=0;i<=n;++i)out[i]=(uint8_t)input[i];
}
static void check(int32_t mode,const char *input,
                  leitz_win2018_wifi_password_result expected)
{
    uint16_t key[128];ascii(key,128,input);
    assert(leitz_win2018_check_wifi_password(mode,key,128)==expected);
}
int main(void)
{
    check(0,"",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    assert(leitz_win2018_check_wifi_password(0,NULL,0)==0);
    assert(leitz_win2018_check_wifi_password(1,NULL,0)==
           LEITZ_WIN2018_WIFI_PASSWORD_REQUIRED);
    check(1,"",LEITZ_WIN2018_WIFI_PASSWORD_REQUIRED);
    check(1,"abcde",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(1,"abcdefghijklm",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(1,"0123456789",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(1,"AABBCCDDEEFF00112233445566",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(1,"ZZZZZZZZZZ",LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT);
    check(1,"abcdefff",LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT);
    check(1,"a\nbcd",LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT);
    check(1,"abcde\n",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(1,"0123456789\n",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(1,"abcde\nmore",LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT);
    check(2,"short",LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT);
    check(2,"12345678",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(5,"12345678",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(9,"12345678",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(13,"12345678",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(2,"12345678\n",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(2,"123\n56789",LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT);
    check(-1,"abc",LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    check(-1,"",LEITZ_WIN2018_WIFI_PASSWORD_REQUIRED);
    uint16_t key[70];
    for(size_t i=0;i<64;++i)key[i]='x';
    key[64]=0;
    assert(leitz_win2018_check_wifi_password(2,key,70)==
           LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT);
    key[63]=0;
    assert(leitz_win2018_check_wifi_password(2,key,70)==
           LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    key[7]=0;
    assert(leitz_win2018_check_wifi_password(2,key,70)==
           LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT);
    key[8]=0;
    assert(leitz_win2018_check_wifi_password(2,key,70)==
           LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT);
    key[7]='x';key[8]=0;
    assert(leitz_win2018_check_wifi_password(2,key,70)==
           LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    const uint16_t unicode[]={0x00e4,0x00e4,0x00e4,0x00e4,0x00e4,0};
    assert(leitz_win2018_check_wifi_password(1,unicode,6)==
           LEITZ_WIN2018_WIFI_PASSWORD_VALID);
    const uint16_t no_null[]={'a','b','c'};
    assert(leitz_win2018_check_wifi_password(2,no_null,3)==
           LEITZ_WIN2018_WIFI_PASSWORD_INPUT_ERROR);
    assert(leitz_win2018_check_wifi_password(2,key,0)==
           LEITZ_WIN2018_WIFI_PASSWORD_INPUT_ERROR);
    puts("Windows AddPrinter 2018 OEM WiFi password policy: PASS");
    return 0;
}
