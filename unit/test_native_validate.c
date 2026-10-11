#include "../../driver/src/leitz_protocols.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static void check(unsigned dpi){
 const leitz_protocol_ops *ops=leitz_protocol_get(LEITZ_PROTOCOL_NATIVE);
 assert(ops);
 FILE *f=tmpfile();assert(f);
 void *state=calloc(1,ops->state_size);assert(state);
 leitz_page page={16,5,5,0,0,3,dpi};
 uint8_t zero[16]={0},ink[16]={0};
 ink[0]=ink[2]=ink[5]=ink[7]=ink[8]=1;
 assert(ops->begin(state,f,&page,1)==0);
 assert(ops->row(state,zero,16)==0);
 assert(ops->row(state,ink,16)==0);
 assert(ops->row(state,zero,16)==0);
 assert(ops->row(state,ink,16)==0);
 assert(ops->row(state,zero,16)==0);
 assert(ops->end_page(state,1)==0);
 rewind(f);
 unsigned pages=0,width=0;char why[100]={0};
 assert(ops->validate(f,&pages,&width,why,sizeof why)==0);
 assert(pages==1&&width==16);
 assert(fseek(f,-1,SEEK_END)==0);
 assert(fputc('X',f)=='X');
 rewind(f);
 assert(ops->validate(f,&pages,&width,why,sizeof why)!=0);
 free(state);fclose(f);
}
int main(void){check(300);check(600);puts("PASS native validator/footer (offline)");return 0;}
