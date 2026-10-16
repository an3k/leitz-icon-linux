#include "printer_data.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef struct {int status;size_t needed;uint32_t type;size_t calls;} fixture;
static int query(void *ctx,const uint16_t *key,uint32_t *type,
                 uint8_t *buf,size_t capacity,size_t *needed) {
  fixture *f=ctx;
  assert(key && key[0]=='T' && capacity==128);
  ++f->calls;*type=f->type;*needed=f->needed;
  for(size_t i=0;i<f->needed && i<capacity;++i)
    buf[i]=(uint8_t)(i^0x5a);
  return f->status;
}
int main(void) {
  /* Synthetic registry value, not a documented manufacturer key. */
  const uint16_t key[]={'T','e','s','t',0};
  uint8_t dest[256];memset(dest,0xa5,sizeof dest);
  size_t len=999;uint32_t type=0x12345678u;
  fixture f={0,3,7,0};
  assert(leitz_win2018_retrieve_printer_value(
    key,dest,sizeof dest,&len,&type,query,&f)==0);
  assert(len==3 && type==7 && f.calls==1);
  assert(dest[0]==0x5a && dest[1]==0x5b &&
         dest[2]==0x58 && dest[3]==0xa5);
  f.needed=128;
  assert(leitz_win2018_retrieve_printer_value(
    key,dest,sizeof dest,&len,&type,query,&f)==0);
  assert(len==128 && dest[127]==(uint8_t)(127^0x5a) &&
         dest[128]==0xa5);
  uint8_t before[256];memcpy(before,dest,sizeof dest);
  len=78;type=93;f.needed=129;
  assert(leitz_win2018_retrieve_printer_value(
    key,dest,sizeof dest,&len,&type,query,&f)==-1);
  assert(len==78 && type==93 && !memcmp(dest,before,sizeof dest));
  f.needed=4;f.status=122;
  assert(leitz_win2018_retrieve_printer_value(
    key,dest,sizeof dest,&len,&type,query,&f)==-1);
  assert(len==78 && type==93 && !memcmp(dest,before,sizeof dest));
  f.status=0;
  assert(leitz_win2018_retrieve_printer_value(
    key,dest,2,&len,&type,query,&f)==-1);
  assert(len==78 && type==93 && !memcmp(dest,before,sizeof dest));
  f.needed=0;
  assert(leitz_win2018_retrieve_printer_value(
    key,NULL,0,&len,&type,query,&f)==0);
  assert(len==0);
  assert(leitz_win2018_retrieve_printer_value(
    NULL,dest,sizeof dest,&len,&type,query,&f)==-1);
  assert(leitz_win2018_retrieve_printer_value(
    key,dest,sizeof dest,NULL,&type,query,&f)==-1);
  puts("Windows 2018 PrintHelper RetrieveStatus spooler data: PASS");
  return 0;
}
