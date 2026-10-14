#define _POSIX_C_SOURCE 200809L
#include "original_raster_ppd_options.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

typedef struct {
    const char *keys[6];
    const char *values[6];
    unsigned visited;
} mock;
static const char *keys[]={"Resolution","PrintQuality","DebugMode","PrintThroughPRN","OptimizationLevel","LabelCutter"};
static const char *chosen(void *ctx,const char *name){
    mock *m=ctx;
    assert(m->visited<6);
    assert(strcmp(name,keys[m->visited])==0);
    ++m->visited;
    for(unsigned i=0;i<6;++i)if(m->keys[i] && strcmp(name,m->keys[i])==0)return m->values[i];
    return NULL;
}
static classic_original_ppd_options_io setup(mock *m){classic_original_ppd_options_io io={m,chosen};return io;}
static void set(mock*m,unsigned idx,const char*value){m->keys[idx]=keys[idx];m->values[idx]=value;}
static uint32_t get(const unsigned char*d,unsigned offset){return classic_original_raster_option_u32(d,offset);}
/* The quality lookup is covered separately with all 15 OEM matrix choices;
 * this stub here isolates the caller's original PPD choice ordering. */
int classic_raster_quality_selector(const char *name){
    static const char* names[]={"Bayer8x8","Bayer8x8SuperCell1","Bayer8x8SuperCell2","SuperCell1","SuperCell2","SuperCell3","CUPSThresold64x64","Draft","SuperCell","Bayer4x4","Bayer14x14","Clustered8x8","Dispersed8x8","Nasik4x4","Dithering3x3"};
    for(int i=0;i<15;++i)if(strcasecmp(name,names[i])==0)return i;
    return -1;
}
int main(void){
    FILE *capture=tmpfile();assert(capture);
    int saved=dup(fileno(stderr));assert(saved>=0);
    assert(dup2(fileno(capture),fileno(stderr))>=0);
    unsigned char d[OEM_DESCRIPTOR_SIZE];
    mock m={0};classic_original_ppd_options_io io=setup(&m);
    classic_original_raster_initialize_options(d);
    assert(get(d,OEM_OPTIMIZATION_OFFSET)==1);
    assert(get(d,OEM_HIGH_RESOLUTION_OFFSET)==0);
    assert(get(d,OEM_PRINT_QUALITY_OFFSET)==0);
    classic_original_raster_select_options(d,&io);
    assert(m.visited==6);
    assert(get(d,OEM_OPTIMIZATION_OFFSET)==1);
    classic_original_raster_report_options(d,&io);


    memset(&m,0,sizeof m);io=setup(&m);
    set(&m,0,"600DPI");set(&m,1,"SUPERCELL3");set(&m,2,"PrintToFile");set(&m,3,"ON");set(&m,4,"3");set(&m,5,"ON");
    classic_original_raster_initialize_options(d);
    classic_original_raster_select_options(d,&io);
    assert(get(d,OEM_HIGH_RESOLUTION_OFFSET)==1);
    assert(get(d,OEM_PRINT_QUALITY_OFFSET)==5);
    assert(get(d,OEM_DEBUG_MODE_OFFSET)==3);
    assert(get(d,OEM_PRINT_THROUGH_PRN_OFFSET)==1);
    assert(get(d,OEM_OPTIMIZATION_OFFSET)==3);
    assert(get(d,OEM_CUT_EVERY_LABEL_OFFSET)==1);
    classic_original_raster_report_options(d,&io);

    for(unsigned n=0;n<15;++n){
      static const char* names[]={"Bayer8x8","Bayer8x8SuperCell1","Bayer8x8SuperCell2","SuperCell1","SuperCell2","SuperCell3","CUPSThresold64x64","Draft","SuperCell","Bayer4x4","Bayer14x14","Clustered8x8","Dispersed8x8","Nasik4x4","Dithering3x3"};
      memset(&m,0,sizeof m);io=setup(&m);set(&m,1,names[n]);
      classic_original_raster_initialize_options(d);classic_original_raster_select_options(d,&io);
      assert(get(d,OEM_PRINT_QUALITY_OFFSET)==n);
    }
    /* Unknown quality should not change prior selection. */
    memset(&m,0,sizeof m);io=setup(&m);set(&m,1,"UnknownQuality");
    classic_original_raster_initialize_options(d);
    { uint32_t q=11;memcpy(d+OEM_PRINT_QUALITY_OFFSET,&q,sizeof q); }
    classic_original_raster_select_options(d,&io);
    assert(get(d,OEM_PRINT_QUALITY_OFFSET)==11);
    /* Case-sensitive debug and ON; unsigned >=4 and negative opt reset 0. */
    memset(&m,0,sizeof m);io=setup(&m);set(&m,2,"debug");set(&m,3,"on");set(&m,4,"-1");set(&m,5,"on");
    classic_original_raster_initialize_options(d);classic_original_raster_select_options(d,&io);
    assert(get(d,OEM_DEBUG_MODE_OFFSET)==0 && get(d,OEM_PRINT_THROUGH_PRN_OFFSET)==0);
    assert(get(d,OEM_OPTIMIZATION_OFFSET)==0 && get(d,OEM_CUT_EVERY_LABEL_OFFSET)==0);
    /* Pure application does not destroy unrelated descriptor fields. */
    memset(d,0xa5,sizeof d);
    memset(&m,0,sizeof m);io=setup(&m);
    classic_original_raster_select_options(d,&io);
    assert(d[0x30]==0xa5 && d[0x1050]==0xa5);
    assert(fflush(stderr)==0);
    assert(dup2(saved,fileno(stderr))>=0);assert(close(saved)==0);
    rewind(capture);
    char reports[4096]={0};size_t r=fread(reports,1,sizeof(reports)-1,capture);reports[r]=0;
    assert(fclose(capture)==0);
    assert(strstr(reports,"\nraster2printer.c:249:raster2printer(): options: resolution:low; dbg-mod:0; opt-lvl:1; cut-every:no")!=NULL);
    assert(strstr(reports,"\nraster2printer.c:142:raster2printer(): PrintQuality choice->choice = SUPERCELL3")!=NULL);
    assert(strstr(reports,"\nraster2printer.c:249:raster2printer(): options: resolution:hi; dbg-mod:3; opt-lvl:3; cut-every:yes")!=NULL);
    puts("macOS classic OEM six PPD choice decisions verified");
    return 0;
}
