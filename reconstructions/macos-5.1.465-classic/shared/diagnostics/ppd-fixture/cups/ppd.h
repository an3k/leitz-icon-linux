#ifndef OEM_MACOS_CLASSIC_PPD_OFFLINE_FIXTURE_H
#define OEM_MACOS_CLASSIC_PPD_OFFLINE_FIXTURE_H
/* Mock field layout ONLY for C source testing. Never used on macOS/printing. */
#include <stddef.h>
typedef enum {PPD_CS_CMYK,PPD_CS_CMY,PPD_CS_GRAY,PPD_CS_RGB,PPD_CS_RGBK,PPD_CS_N} ppd_cs_t;
typedef enum {PPD_UI_BOOLEAN,PPD_UI_PICKONE,PPD_UI_PICKMANY} ppd_ui_t;
typedef enum {PPD_ORDER_ANY,PPD_ORDER_DOCUMENT,PPD_ORDER_EXIT,PPD_ORDER_JCL,PPD_ORDER_PAGE,PPD_ORDER_PROLOG} ppd_section_t;
typedef struct {int marked; char *choice; char *code;} ppd_choice_t;
typedef struct {char *keyword; int num_choices; char *defchoice; ppd_ui_t ui; ppd_section_t section; ppd_choice_t *choices;} ppd_option_t;
typedef struct {char *name; int num_options,num_subgroups; ppd_option_t *options;} ppd_group_t;
typedef struct {char *name,*value;} ppd_attr_t;
typedef struct {int marked;char *name;float width,length,left,bottom,right,top;} ppd_size_t;
typedef struct {
 int num_filters,num_attrs,num_groups,num_sizes; char **filters;
 int language_level,color_device,variable_sizes,accurate_screens,contone_only,landscape,model_number,manual_copies,throughput;
 char *jcl_begin,*jcl_ps,*jcl_end,*lang_encoding,*lang_version,*modelname,*ttrasterizer,*manufacturer,*product,*nickname,*shortnickname,*protocols,*pcfilename;
 int cur_attr;ppd_cs_t colorspace;ppd_attr_t **attrs;ppd_group_t *groups;ppd_size_t *sizes;
} ppd_file_t;
ppd_file_t *ppdOpenFile(const char *);
void ppdClose(ppd_file_t *);
ppd_size_t *ppdPageSize(ppd_file_t *,const char *);
float ppdPageWidth(ppd_file_t *,const char *);
float ppdPageLength(ppd_file_t *,const char *);
#endif
