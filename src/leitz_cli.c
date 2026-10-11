/* Standalone offline generator and validator. PDF processing is not included. */
#include "leitz_jobs.h"
#include "leitz_status.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/stat.h>
static int token(FILE *f,char *s,size_t max){
 int c;do{c=fgetc(f);if(c=='#'){while(c!='\n'&&c!=EOF)c=fgetc(f);}}while(c!=EOF&&isspace((unsigned char)c));
 if(c==EOF)return 0;
 size_t n=0;do{if(n+1>=max)return 0;s[n++]=(char)c;c=fgetc(f);}while(c!=EOF&&!isspace((unsigned char)c));
 s[n]=0;return 1;
}
static int cp(FILE *in,FILE *out){char b[8192];size_t n;while((n=fread(b,1,sizeof b,in)))if(fwrite(b,1,n,out)!=n)return -1;return ferror(in)?-1:0;}
int main(int argc,char **argv){
 if(argc==3&&(!strcmp(argv[1],"validate")||
                !strcmp(argv[1],"validate-native")||
                !strcmp(argv[1],"validate-auto"))){
  const int automatic=!strcmp(argv[1],"validate-auto");
  const int native=!strcmp(argv[1],"validate-native");
  const leitz_protocol selected=native?LEITZ_PROTOCOL_NATIVE:LEITZ_PROTOCOL_CURRENT;
  if(!automatic && !leitz_protocol_get(selected)){
   fprintf(stderr,"ERROR: Requested protocol module not installed\n");return 2;
  }
  FILE *f=fopen(argv[2],"rb");if(!f){perror(argv[2]);return 1;}
  char msg[160]={0};unsigned pages=0,width=0;int ok=-1;
  const leitz_protocol order[]={LEITZ_PROTOCOL_CURRENT,LEITZ_PROTOCOL_NATIVE};
  for(unsigned i=0;i<(automatic?2u:1u);i++){
   leitz_protocol type=automatic?order[i]:selected;
   const leitz_protocol_ops *ops=leitz_protocol_get(type);
   if(!ops)continue;
   if(fseek(f,0,SEEK_SET)){perror("Rewinding spool");break;}
   msg[0]=0;pages=width=0;
   if(ops->validate(f,&pages,&width,msg,sizeof msg)==0){
    ok=0;
    if(automatic)
     fprintf(stderr,"OK: %s document; pages=%u; max_width=%u\n",ops->name,pages,width);
    else if(type==LEITZ_PROTOCOL_NATIVE)
     fprintf(stderr,"OK: native Linux 1.0.3 document; pages=%u; max_width=%u\n",pages,width);
    else fprintf(stderr,"%s\n",msg);
    break;
   }
  }
  if(fclose(f)){perror("Closing input");return 1;}
  if(ok){fprintf(stderr,"ERROR: No installed protocol accepted the document: %s\n",msg);return 1;}
  printf("MAX_WIDTH=%u\n",width);
  return 0;
 }
 if(argc==4 && !strcmp(argv[1],"decode-status")) {
  /* Decodes a regular-file snapshot; never opens a device or sends commands. */
  leitz_reply_kind kind;
  if(leitz_reply_kind_from_name(argv[2],&kind)) {
    fprintf(stderr,"ERROR: Use legacy, v3, lines, or version\n");return 2;
  }
  struct stat info;
  if(stat(argv[3],&info) || !S_ISREG(info.st_mode) ||
     info.st_size<1 || info.st_size>(long)LEITZ_REPLY_MAX) {
    fprintf(stderr,"ERROR: Requires a 1..1024-byte regular reply file\n");return 1;
  }
  FILE *input=fopen(argv[3],"rb");
  if(!input){perror(argv[3]);return 1;}
  uint8_t bytes[LEITZ_REPLY_MAX+1];
  size_t n=fread(bytes,1,sizeof bytes,input);
  int io_bad=ferror(input),close_bad=fclose(input);
  if(io_bad || close_bad || n!=(size_t)info.st_size || n>LEITZ_REPLY_MAX) {
    fprintf(stderr,"ERROR: Cannot read complete reply snapshot\n");return 1;
  }
  leitz_reply parsed;
  if(leitz_parse_reply(kind,bytes,n,&parsed)) {
    fprintf(stderr,"ERROR: Malformed or unsupported reply snapshot\n");return 1;
  }
  printf("REPLY_BYTES=%zu\nINTERPRETATION=%s\n",n,parsed.interpretation);
  if(kind==LEITZ_REPLY_PRINTED_LINES)
    printf("PRINTED_LINES_CANDIDATE=%u\n",parsed.printed_lines);
  else if(kind==LEITZ_REPLY_VERSION)
    printf("VERSION_TEXT=%s\n",parsed.version);
  else
    printf("STATUS_HEX=0x%02X\nAMBIGUOUS=%s\nDEVICE_VERIFIED=NO\n",
           (unsigned)parsed.status_byte,parsed.ambiguous?"YES":"NO");
  return 0;
 }
 if(argc!=4||strcmp(argv[1],"pgm")){
  fprintf(stderr,"Usage: leitz-cli validate file.bin | validate-native file.bin | validate-auto file.bin | pgm image.pgm output.bin | decode-status legacy|v3|lines|version reply.bin\n");return 2;
 }
 FILE *in=fopen(argv[2],"rb");if(!in){perror(argv[2]);return 1;}
 char word[80];unsigned w=0,h=0,max=0;int err=0;
 if(!token(in,word,sizeof word)||strcmp(word,"P5") ||
    !token(in,word,sizeof word)||(w=(unsigned)atoi(word))<1||w>LEITZ_MAX_WIDTH ||
    !token(in,word,sizeof word)||(h=(unsigned)atoi(word))<1||h+363>LEITZ_MAX_ROWS ||
    !token(in,word,sizeof word)||(max=(unsigned)atoi(word))!=255){
     fprintf(stderr,"ERROR: Requires 8-bit P5 PGM, width <=960, max 64000 print rows\n");fclose(in);return 1;
 }
 FILE *temp=tmpfile();if(!temp){perror("tmpfile");fclose(in);return 1;}
 leitz_job *job=leitz_job_create(temp,LEITZ_PROTOCOL_CURRENT,1);
 if(!job){fprintf(stderr,"ERROR: Current protocol module unavailable\n");fclose(in);fclose(temp);return 1;}
 err=leitz_job_begin_page(job,w,h+363,h,182,181,65535,600);
 uint8_t *gray=malloc(w),*bits=malloc(w);
 if(!gray||!bits)err=1;
 for(unsigned y=0;!err&&y<h;y++){
  if(fread(gray,1,w,in)!=w){err=1;break;}
  for(unsigned x=0;x<w;x++)bits[x]=gray[x]<128;
  if(leitz_job_row(job,bits))err=1;
 }
 if(!err && (leitz_job_end_page(job)||leitz_job_finish(job)))err=1;
 if(!err){
  char msg[160]={0};
  if(leitz_job_validate(job,msg,sizeof msg))err=1;
 }
 if(!err){
  FILE *out=fopen(argv[3],"wb");if(!out){perror(argv[3]);err=1;}
  else{rewind(temp);if(cp(temp,out)||fclose(out))err=1;}
 }
 leitz_job_destroy(job);
 fclose(in);fclose(temp);free(gray);free(bits);
 if(err){fprintf(stderr,"ERROR: No validated output produced\n");return 1;}
 fprintf(stderr,"Generated offline Leitz raster job: %s\n",argv[3]);return 0;
}
