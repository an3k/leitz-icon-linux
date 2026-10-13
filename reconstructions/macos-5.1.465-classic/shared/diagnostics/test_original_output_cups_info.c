#define _POSIX_C_SOURCE 200809L
#include <cups/ppd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void output_cups_info(const char *,FILE *);
static ppd_file_t p;
static char *filters[] = {"application/vnd.cups-raster 0 raster2printer"};
static ppd_attr_t att={"cupsVersion","2.4"};
static ppd_attr_t *attrs[] = {&att};
static ppd_choice_t choices[] = {{0,"OFF","<</PrinterCut false>>setpagedevice"},{1,"ON","<</PrinterCut true>>setpagedevice"}};
static ppd_option_t opts[]={{"LabelCutter",2,"OFF",PPD_UI_PICKONE,PPD_ORDER_PAGE,choices}};
static ppd_group_t groups[]={{"General",1,0,opts}};
static ppd_size_t sizes[]={{1,"StandardAddressLabel",300.0f,200.0f,5.0f,4.0f,6.0f,7.0f}};
ppd_file_t *ppdOpenFile(const char *path){assert(strcmp(path,"manufacturer.ppd")==0);p=(ppd_file_t){0};p.num_filters=1;p.filters=filters;p.language_level=3;p.color_device=0;p.manufacturer="Leitz";p.modelname="Icon";p.colorspace=PPD_CS_RGB;p.num_attrs=1;p.attrs=attrs;p.num_groups=1;p.groups=groups;p.num_sizes=1;p.sizes=sizes;return &p;}
void ppdClose(ppd_file_t *a){assert(a==&p);}
ppd_size_t *ppdPageSize(ppd_file_t *a,const char *name){assert(a==&p);return strcmp(name,"StandardAddressLabel")==0?sizes:NULL;}
float ppdPageWidth(ppd_file_t *a,const char *name){return ppdPageSize(a,name)->width;}
float ppdPageLength(ppd_file_t *a,const char *name){return ppdPageSize(a,name)->length;}
static void check(const char *s,const char *part){if(!strstr(s,part)){fprintf(stderr,"missing substring: %s\n",part);abort();}}
int main(void){
 FILE *f=tmpfile();assert(f);
 output_cups_info("manufacturer.ppd",f);
 assert(fflush(f)==0);assert(fseek(f,0,SEEK_END)==0);long sz=ftell(f);assert(sz>100 && sz<20000);rewind(f);
 char *s=calloc((size_t)sz+1,1);assert(s);assert(fread(s,1,(size_t)sz,f)==(size_t)sz);
 check(s,"raster_c_functions.c:205:output_cups_info(): output cups info, cups file:'manufacturer.ppd'");
 check(s,"raster_c_functions.c:214:output_cups_info(): filter 0:");
 check(s,"raster_c_functions.c:244:output_cups_info(): attr[00]: 'cupsVersion'='2.4'");
 check(s,"raster_c_functions.c:248:output_cups_info(): colorspace: PPD_CS_RGB");
 check(s,"raster_c_functions.c:267:output_cups_info(): group[0]: 'General'; num_options:1; num_subgroups:0");
 check(s,"raster_c_functions.c:271:output_cups_info(): option[0]: 'LabelCutter'");
 check(s,"raster_c_functions.c:296:output_cups_info(): choice[00]: - 'OFF'");
 check(s,"raster_c_functions.c:311:output_cups_info(): marked +, name 'StandardAddressLabel'");
 check(s,"raster_c_functions.c:312:output_cups_info(): WxH: 300.000000x200.000000");
 check(s,"raster_c_functions.c:315:output_cups_info(): ImageArea: 289.000000x189.000000");
 check(s,"raster_c_functions.c:322:output_cups_info(): Sizes[1]");
 check(s,"raster_c_functions.c:340:output_cups_info():");
 free(s);assert(fclose(f)==0);
 puts("Classic output_cups_info manufacturer line/field/PPD branch checks passed");return 0;
}
