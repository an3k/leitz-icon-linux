#include "wifi_password.h"

static int hex_digit(uint16_t c)
{
    return (c>='0' && c<='9') || (c>='A' && c<='F') ||
           (c>='a' && c<='f');
}

/* The .NET default Regex '.' matches one UTF-16 code unit except LF;
 * the '$' anchor also matches just before an optional trailing LF.
 * Reject LF in the actual matched prefix.
 */
static size_t anchored_length(const uint16_t *key,size_t n)
{
    if(n && key[n-1]=='\n')--n;
    for(size_t i=0;i<n;++i)if(key[i]=='\n')return SIZE_MAX;
    return n;
}

leitz_win2018_wifi_password_result leitz_win2018_check_wifi_password(
    int32_t security_mode,const uint16_t *key,size_t capacity)
{
    /* OEM MethodDef 285: mode zero returns true before testing
     * String.IsNullOrEmpty(password). */
    if(security_mode==0)return LEITZ_WIN2018_WIFI_PASSWORD_VALID;
    if(!key)return LEITZ_WIN2018_WIFI_PASSWORD_REQUIRED;
    if(!capacity)return LEITZ_WIN2018_WIFI_PASSWORD_INPUT_ERROR;
    size_t n=0;
    while(n<capacity && key[n])++n;
    if(n==capacity)return LEITZ_WIN2018_WIFI_PASSWORD_INPUT_ERROR;
    if(!n)return LEITZ_WIN2018_WIFI_PASSWORD_REQUIRED;

    const size_t match=anchored_length(key,n);
    if(security_mode==1) {
        /* Original .cctor creates five Regex instances in order:
         * ^[0-9a-fA-F]{10}$, ^.{5}$,
         * ^[0-9a-fA-F]{26}$, ^.{13}$, ^.{8,63}$.
         * The first four are tried for WEP (security mode 1). */
        if(match==5 || match==13)
            return LEITZ_WIN2018_WIFI_PASSWORD_VALID;
        if(match==10 || match==26) {
            for(size_t i=0;i<match;++i)
                if(!hex_digit(key[i]))
                    return LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT;
            return LEITZ_WIN2018_WIFI_PASSWORD_VALID;
        }
        return LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT;
    }
    /* Original checks mode > 1 (signed), not only the codes 5,9,13
     * which its independent GetSecurityModeString labels as WPA. */
    if(security_mode>1)
        return (match>=8 && match<=63)
            ? LEITZ_WIN2018_WIFI_PASSWORD_VALID
            : LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT;
    /* Negative mode: after nonempty check OEM returns true. */
    return LEITZ_WIN2018_WIFI_PASSWORD_VALID;
}
