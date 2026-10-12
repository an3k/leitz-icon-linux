#include "mac_classic_process_page.h"
#include "mac_classic_command_core.h"
#include "mac_classic_raster_boundaries.h"
#include "mac_classic_dither.h"
#include <assert.h>
#include <string.h>
int main(void) {
  uint8_t b[124];
  for (int i = 0; i < 3; i++) {
    assert(mac_classic_encode_query(mac_classic_queries[i].command,b) == 2);
    assert(b[0] == 0x1b && b[1] == mac_classic_queries[i].command);
  }
  assert(mac_classic_encode_end_of_job(b) == 2);
  assert(b[0] == 0x1b && b[1] == 'Z');
  assert(mac_classic_decode_a(0x03) == MAC_CLASSIC_READY);
  assert(mac_classic_decode_a(0x20) == MAC_CLASSIC_MEDIA_EMPTY);
  assert(mac_classic_decode_a(0x30) == MAC_CLASSIC_MEDIA_JAM);
  assert(mac_classic_decode_a(0x40) == MAC_CLASSIC_GENERAL_ERROR);
  assert(mac_classic_decode_a(0x23) == MAC_CLASSIC_UNKNOWN);
  assert(strcmp(mac_classic_status_reason(MAC_CLASSIC_READY),"com.leitz.ready")==0);
  assert(mac_classic_start_document(0,b)==122);
  for (int i=0; i<121; ++i) assert(b[i]==0x1b);
  assert(b[121]=='@');
  assert(mac_classic_start_document(1,b)==124);
  assert(b[122]==0x1b && b[123]=='i');
  assert(mac_classic_end_page(0,b)==2);
  assert(memcmp(b,"\033G",2)==0);
  assert(mac_classic_end_page(1,b)==4);
  assert(memcmp(b,"\033G\033E",4)==0);
  assert(mac_classic_end_document(0,0,b)==6);
  assert(memcmp(b,"\033E\033@\033Z",6)==0);
  assert(mac_classic_end_document(0,1,b)==4);
  assert(memcmp(b,"\033@\033Z",4)==0);
  assert(mac_classic_end_document(1,0,b)==4);
  assert(memcmp(b,"\033@\033Z",4)==0);
  uint8_t header[11];
  assert(mac_classic_start_page(960,300,0,header)==11);
  const uint8_t expected300[11]={0x1b,'L',0x01,0x4a,0x1b,'w',0x03,0xc0,0x1b,'B',0};
  assert(memcmp(header,expected300,11)==0);
  assert(mac_classic_start_page(960,600,1,header)==11);
  assert(memcmp(header,expected300,11)==0);
  assert(mac_classic_start_page(960,601,1,header)==11);
  assert(header[2]==0x01 && header[3]==0x4a);
  mac_classic_quality info;
  const unsigned widths[15]={8,10,12,18,12,6,64,0,8,4,14,8,8,4,3};
  const double scales[15]={64,64,64,256,70,20,255,0,31,16,255,40,45,16,8};
  const char *quality_choices[15]={
      "Bayer8x8", "Bayer8x8SuperCell1", "Bayer8x8SuperCell2",
      "SuperCell1", "SuperCell2", "SuperCell3", "CUPSThresold64x64",
      "Draft", "SuperCell", "Bayer4x4", "Bayer14x14", "Clustered8x8",
      "Dispersed8x8", "Nasik4x4", "Dithering3x3"
  };
  assert(mac_classic_quality_from_ppd("SuperCELL2")==4);
  assert(mac_classic_quality_from_ppd("Unsupported")<0);
  for(unsigned mode=0;mode<15;++mode){
    assert(mac_classic_quality_from_ppd(quality_choices[mode])==(int)mode);
    if(mode==7){assert(mac_classic_quality_info(mode,&info)<0);continue;}
    assert(mac_classic_quality_info(mode,&info)==0);
    assert(info.matrix_width==widths[mode]);
    assert(info.matrix_divisor==scales[mode]);
  }
  uint8_t rgb[48]={0},onebit[2]={0};
  assert(mac_classic_dither_rgb24(0,rgb,sizeof(rgb),2,24,onebit,sizeof(onebit))==0);
  assert(onebit[0]==0xff && onebit[1]==0xff);
  memset(rgb,255,sizeof(rgb));
  assert(mac_classic_dither_rgb24(0,rgb,sizeof(rgb),2,24,onebit,sizeof(onebit))==0);
  assert(onebit[0]==0 && onebit[1]==0);
  assert(mac_classic_dither_rgb24(7,rgb,sizeof(rgb),2,24,onebit,sizeof(onebit))<0);
  assert(mac_classic_dither_rgb24(0,rgb,sizeof(rgb),2,23,onebit,sizeof(onebit))<0);
  assert(mac_classic_dither_rgb24(0,rgb,sizeof(rgb),3,24,onebit,sizeof(onebit))<0);
  /* Mach-O ProcessPage mode 2/3: independently verified byte framing. */
  mac_classic_raster_row st;
  uint8_t dst[256]; size_t used=0;
  const uint8_t blank[4]={0,0,0,0};
  const uint8_t dense[4]={0,0xff,0xff,0xff};
  const uint8_t sparse[4]={0,0,0xff,0};
  assert(mac_classic_row_init(&st,2,4)==0);
  assert(mac_classic_row_encode(&st,blank,dst,sizeof dst,&used)==0 && used==0);
  assert(mac_classic_row_encode(&st,blank,dst,sizeof dst,&used)==0 && used==0);
  assert(mac_classic_row_encode(&st,dense,dst,sizeof dst,&used)==0);
  const uint8_t expected_rle[]={0x1b,'f',0,2,0x1b,'B',1,0x1b,'D',3,0x17,0x97};
  assert(used==sizeof expected_rle && memcmp(dst,expected_rle,used)==0);
  /* Compressed stream >= raw width: the binary falls back to SYN/raw. */
  const uint8_t mixed[4]={0,0x55,0x55,0};
  assert(mac_classic_row_encode(&st,mixed,dst,sizeof dst,&used)==0);
  const uint8_t expected_raw[]={0x1b,'B',1,0x1b,'D',2,0x16,0x55,0x55};
  assert(used==sizeof expected_raw && memcmp(dst,expected_raw,used)==0);
  assert(mac_classic_row_init(&st,3,4)==0);
  assert(mac_classic_row_encode(&st,sparse,dst,sizeof dst,&used)==0);
  const uint8_t trimmed[]={0x1b,'B',2,0x1b,'D',1,0x16,0xff};
  assert(used==sizeof trimmed && memcmp(dst,trimmed,used)==0);
  assert(mac_classic_row_encode(&st,blank,dst,sizeof dst,&used)==0 && used==0);
  assert(mac_classic_pending_trailing_blank(&st)==1);
  assert(mac_classic_row_init(&st,2,121)<0);
  assert(mac_classic_row_init(&st,0,4)<0);
  assert(mac_classic_row_init(&st,2,4)==0);
  assert(mac_classic_row_encode(&st,dense,dst,2,&used)<0);
  return 0;
}
