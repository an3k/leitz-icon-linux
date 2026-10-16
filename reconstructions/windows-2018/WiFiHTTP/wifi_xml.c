#include "wifi_xml.h"
#include <expat.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define OEM_XML_MAX_INPUT 65536u
#define FIELD_COUNT 5
#define TEXT_LIMIT 255u
typedef struct {
    XML_Parser parser;
    int mode; /* 0=scan, 1=network */
    unsigned depth,field_depth;
    int active,failed,has_row;
    unsigned seen;
    char text[FIELD_COUNT][TEXT_LIMIT+1];
    size_t text_len[FIELD_COUNT];
} model;
static const char *scan_fields[]={"ScanInProgress","NumScanResults","MySsid"};
static const char *row_fields[]={"Valid","Name","SecurityMode","NetworkType","Strength"};

static void fail(model *s)
{
    s->failed=1;
    XML_StopParser(s->parser,XML_FALSE);
}
static void start(void *user,const XML_Char *name,const XML_Char **attributes)
{
    model*s=user;
    (void)attributes;
    if(s->failed)return;
    ++s->depth;
    if(s->depth>6 || s->active>=0){fail(s);return;}
    if(s->depth==1 && strcmp(name,"response")){fail(s);return;}
    if(s->mode && s->depth==2 && strcmp(name,"bss")){fail(s);return;}
    if(s->mode && s->depth==3){
        if(s->has_row){fail(s);return;}
        s->has_row=1;
    }
    unsigned expected=s->mode?4u:2u;
    if(s->depth!=expected)return;
    const char *const *fields=s->mode?row_fields:scan_fields;
    unsigned n=s->mode?5u:3u;
    for(unsigned i=0;i<n;++i)if(!strcmp(name,fields[i])){
        if(s->seen&(1u<<i)){fail(s);return;}
        s->seen|=1u<<i;
        s->active=(int)i;s->field_depth=s->depth;
        return;
    }
}
static void end(void *user,const XML_Char *name)
{
    model*s=user;
    (void)name;
    if(s->active>=0 && s->depth==s->field_depth)s->active=-1;
    if(s->depth)--s->depth;
}
static void chars(void *user,const XML_Char *p,int len)
{
    model*s=user;
    if(s->active<0 || s->failed || len<=0)return;
    unsigned field=(unsigned)s->active;
    if((size_t)len>TEXT_LIMIT-s->text_len[field]){fail(s);return;}
    memcpy(s->text[field]+s->text_len[field],p,(size_t)len);
    s->text_len[field]+=(size_t)len;
    s->text[field][s->text_len[field]]=0;
}
static void doctype(void *user,const XML_Char *name,
                    const XML_Char *sys,const XML_Char *pub,int internal)
{
    (void)name;(void)sys;(void)pub;(void)internal;
    fail(user);
}
static int parse(const char *xml,size_t length,int mode,model *s)
{
    if(!xml||!length||length>OEM_XML_MAX_INPUT||length>(size_t)INT_MAX)
        return -1;
    memset(s,0,sizeof *s);
    s->mode=mode;s->active=-1;
    s->parser=XML_ParserCreate(NULL);
    if(!s->parser)return -1;
    XML_SetUserData(s->parser,s);
    XML_SetElementHandler(s->parser,start,end);
    XML_SetCharacterDataHandler(s->parser,chars);
    XML_SetStartDoctypeDeclHandler(s->parser,doctype);
    XML_SetParamEntityParsing(s->parser,XML_PARAM_ENTITY_PARSING_NEVER);
    int ok=XML_Parse(s->parser,xml,(int)length,XML_TRUE)==XML_STATUS_OK;
    XML_ParserFree(s->parser);s->parser=NULL;
    return ok&&!s->failed&&s->depth==0?0:-1;
}
static int number(const char *s,int32_t *out)
{
    /* Managed Integer.Parse accepts signed decimal integers and
     * surrounding whitespace. Reject overflow and partial parsing. */
    while(isspace((unsigned char)*s))++s;
    if(!*s)return -1;
    char*end=NULL;
    long long n=strtoll(s,&end,10);
    if(!end||end==s||n<INT32_MIN||n>INT32_MAX)return -1;
    while(isspace((unsigned char)*end))++end;
    if(*end)return -1;
    *out=(int32_t)n;
    return 0;
}
int leitz_win2018_parse_scan_xml(const char *xml,size_t length,
                                 leitz_win2018_scan_xml *out)
{
    if(!out)return -1;
    model s;
    if(parse(xml,length,0,&s)||!(s.seen&1u))return -1;
    leitz_win2018_scan_xml result={0};
    result.in_progress=!strcmp(s.text[0],"1");
    if(!result.in_progress){
        if((s.seen&6u)!=6u)return -1;
        int32_t count;
        if(number(s.text[1],&count)||count<0)return -1;
        result.number_of_results=(uint32_t)count;
        memcpy(result.my_ssid,s.text[2],s.text_len[2]+1);
    }
    *out=result;
    return 0;
}
int leitz_win2018_parse_bss_xml(const char *xml,size_t length,
                                leitz_win2018_bss_xml *out)
{
    if(!out)return -1;
    model s;
    if(parse(xml,length,1,&s)||!s.has_row||!(s.seen&1u))return -1;
    leitz_win2018_bss_xml result={0};
    result.valid=!strcmp(s.text[0],"1");
    if(result.valid){
        if((s.seen&31u)!=31u||
           number(s.text[2],&result.security_mode)||
           number(s.text[3],&result.network_type)||
           number(s.text[4],&result.strength))return -1;
        memcpy(result.name,s.text[1],s.text_len[1]+1);
    }
    *out=result;
    return 0;
}
