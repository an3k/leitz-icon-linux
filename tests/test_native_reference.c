#include "../src/leitz_protocol.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static void check(unsigned dpi){
 const leitz_protocol_ops *ops=leitz_protocol_get(LEITZ_PROTOCOL_NATIVE_LINUX_103);
 assert(ops);
 FILE *f=tmpfile();assert(f);
 void *state=calloc(1,ops->state_size);assert(state);
 leitz_page page={16,4,4,0,0,2,dpi};
 assert(ops->begin(state,f,&page,1)==0);
 const uint8_t white[16]={0};
 uint8_t ink[16]={0};
 ink[0]=ink[7]=ink[9]=ink[14]=1;
 assert(ops->row(state,white,16)==0);
 assert(ops->row(state,ink,16)==0);
 assert(ops->row(state,white,16)==0);
 assert(ops->row(state,white,16)==0);
 assert(ops->end_page(state,1)==0);
 assert(ops->end_page(state,1)!=0);
 assert(fseek(f,0,SEEK_END)==0);
 long len=ftell(f);
 assert(len==4+120+11+4+6+4);
 rewind(f);
 uint8_t got[200]={0};
 assert(fread(got,1,(size_t)len,f)==(size_t)len);
 assert(got[0]==27 && got[1]=='@' && got[2]==27 &&
        got[3]==(dpi==600?'i':'h'));
 for(unsigned i=4;i<124;i++)assert(got[i]==27);
 const uint8_t setup[]={27,'L',0,2,27,'w',0,16,27,'B',0};
 assert(memcmp(got+124,setup,sizeof setup)==0);
 const uint8_t body[]={27,'f',0,1,27,'D',2,0x16,0x81,0x42,27,'G',27,'E'};
 assert(memcmp(got+135,body,sizeof body)==0);
 free(state);fclose(f);
}
int main(void){check(300);check(600);puts("PASS original native wire framing at both dpi");return 0;}
