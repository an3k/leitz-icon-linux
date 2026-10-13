/* Original Leitz macOS 5.1.465 classic _output_cups_info reconstruction.
 * Both verified OEM binaries: raster2printer 0x100001ef0..0x100003c60;
 * command2leitz 0x100002520..0x100004290. Both have 1372 identical
 * structural assembly records and the same 108 external calls.
 * C control flow comes from the original vendor Linux 1.0.3 implementation;
 * every diagnostic and PPD field is verified against the classic Mach-O.
 * The original body, including unchecked values and unsafe sprintf offsets,
 * is preserved; there are no behavioral improvements.
 */
#include <stdio.h>
#include <string.h>
#include <cups/ppd.h>

#define SHOULD_LOG 1
#define FILE_NAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LLog_FD(fmt, ...) \
do { if (SHOULD_LOG) fprintf(fd, "\n%s:%d:%s(): " fmt, FILE_NAME, \
__LINE__, __func__, __VA_ARGS__); } while (0)

/* Four vendor diagnostics originate on the trailing line of a
 * multiline invocation. The macOS compiler recorded these four line
 * constants one higher than the Linux preprocessor does. Reproduce the
 * observed Mach-O line number without changing any control-flow branch. */
#define LLog_FD_NATIVE_LINE(n,fmt,...) do { if (SHOULD_LOG) \
fprintf(fd, "\n%s:%d:%s(): " fmt, FILE_NAME, (n), __func__, __VA_ARGS__); } while (0)

/* The Mach-O first report is line 205; the Linux source has line 154.
 * All other OEM line numbers map via the same 51-line difference,
 * including the multi-line macro expansions. */
#line 203 "raster_c_functions.c"
void output_cups_info(const char *cups_file, FILE *fd)
{
	LLog_FD("output cups info, cups file:'%s'", cups_file);
	ppd_file_t *ppd = ppdOpenFile(cups_file);
	
#define PRINT_PROPERTY_INT(prop_name)		fprintf(fd, "\t"#prop_name"color_device: %d", ppd->prop_name)
#define PRINT_PROPERTY_CHAR(prop_name)		if (ppd->prop_name) fprintf(fd, "\t"#prop_name"color_device: %s", ppd->prop_name)
	
	if (1) {
		for (int i=0; i<ppd->num_filters; ++i)
        {
			LLog_FD("filter %d: '%s' ", i, ppd->filters[i]);
		}
		PRINT_PROPERTY_INT(language_level);
		PRINT_PROPERTY_INT(color_device);
		PRINT_PROPERTY_INT(variable_sizes);
		PRINT_PROPERTY_INT(accurate_screens);
		PRINT_PROPERTY_INT(contone_only);
		PRINT_PROPERTY_INT(landscape);
		PRINT_PROPERTY_INT(model_number);
		PRINT_PROPERTY_INT(manual_copies);
		PRINT_PROPERTY_INT(throughput);
		
		PRINT_PROPERTY_CHAR(jcl_begin);
		PRINT_PROPERTY_CHAR(jcl_ps);
		PRINT_PROPERTY_CHAR(jcl_end);
		PRINT_PROPERTY_CHAR(lang_encoding);
		PRINT_PROPERTY_CHAR(lang_version);
		PRINT_PROPERTY_CHAR(modelname);
		PRINT_PROPERTY_CHAR(ttrasterizer);
		PRINT_PROPERTY_CHAR(manufacturer);
		PRINT_PROPERTY_CHAR(product);
		PRINT_PROPERTY_CHAR(nickname);
		PRINT_PROPERTY_CHAR(shortnickname);
		PRINT_PROPERTY_CHAR(protocols);
		PRINT_PROPERTY_CHAR(pcfilename);
		PRINT_PROPERTY_INT(num_attrs);
		PRINT_PROPERTY_INT(cur_attr);
		
		for (int i=0; i<ppd->num_attrs; ++i)
        {
			LLog_FD("attr[%02d]: '%s'='%s'", i, ppd->attrs[i]->name, ppd->attrs[i]->value);
		}
		
#define CASE(name) 				case name: fprintf(fd, #name); break;
		LLog_FD("%s", "colorspace: ");
		switch (ppd->colorspace) {
				CASE(PPD_CS_CMYK);
				CASE(PPD_CS_CMY);
				CASE(PPD_CS_GRAY);
				CASE(PPD_CS_RGB);
				CASE(PPD_CS_RGBK);
				CASE(PPD_CS_N);
				break;
				
			default:
                LLog_FD("undefined (%d)", ppd->colorspace);
				break;
		}
		fprintf(fd, "");
		
		//	groups
		for (int i=0; i<ppd->num_groups; ++i) {
			LLog_FD_NATIVE_LINE(267,"group[%d]: '%s'; num_options:%d; num_subgroups:%d", i,
					ppd->groups[i].name, ppd->groups[i].num_options, ppd->groups[i].num_subgroups);
			//	options
			for (int j=0; j<ppd->groups[i].num_options; ++j) {
				LLog_FD_NATIVE_LINE(271,"option[%d]: '%s'; num_choices:%d; defchoice:'%s' ", j,
						ppd->groups[i].options[j].keyword, ppd->groups[i].options[j].num_choices, ppd->groups[i].options[j].defchoice);
				switch (ppd->groups[i].options[j].ui) {
						CASE(PPD_UI_BOOLEAN);
						CASE(PPD_UI_PICKONE);
						CASE(PPD_UI_PICKMANY);
					default:
                        LLog_FD("undefined (%d)", ppd->groups[i].options[j].ui);
						break;
				}
				fprintf(fd, "; ");
				switch (ppd->groups[i].options[j].section) {
						CASE(PPD_ORDER_ANY);
						CASE(PPD_ORDER_DOCUMENT);
						CASE(PPD_ORDER_EXIT);
						CASE(PPD_ORDER_JCL);
						CASE(PPD_ORDER_PAGE);
						CASE(PPD_ORDER_PROLOG);
					default:
                        LLog_FD("undefined (%d)", ppd->groups[i].options[j].section);
						break;
				}
				fprintf(fd, ";");
				//	choices = ppd_choice_t[]
				for (int k=0; k<ppd->groups[i].options[j].num_choices; ++k) {
					LLog_FD_NATIVE_LINE(296,"choice[%02d]: %s '%s' :\t '%s'", k,
							ppd->groups[i].options[j].choices[k].marked==0?"-":"+", ppd->groups[i].options[j].choices[k].choice, ppd->groups[i].options[j].choices[k].code);
				}
			}
		}
		
	}
#undef PRINT_PROPERTY_INT
#undef PRINT_PROPERTY_CHAR
#undef CASE
	
	const char * paper_name = "StandardAddressLabel";
	ppd_size_t * size1 = ppdPageSize(ppd, paper_name);
	fprintf(fd, "ppdPageSize (in points) '%s':", paper_name);
	if (size1) {
		LLog_FD_NATIVE_LINE(311,"marked %s, name '%s', width %2.3f, length %2.3f, left %2.3f, bottom %2.3f, right %2.3f, top %2.3f",
				size1->marked?"+":"-", size1->name, size1->width, size1->length, size1->left, size1->bottom, size1->right, size1->top);
		LLog_FD("WxH: %fx%f", ppdPageWidth(ppd, paper_name), ppdPageLength(ppd, paper_name));
		const float w = size1->width - size1->left - size1->right;
		const float h = size1->length - size1->top - size1->bottom;
		LLog_FD("ImageArea: %fx%f", w, h);
	}
	else
    {
		LLog_FD("no paper named '%s'", paper_name);
	}
	
	LLog_FD("Sizes[%d]", ppd->num_sizes);
	char str_buf[1024];
	for (int i=0; i<ppd->num_sizes; ++i) {
		memset(str_buf, ' ', sizeof(str_buf));
		ppd_size_t ppd_size = ppd->sizes[i];
		
		int offset = sprintf(str_buf, "%s ", ppd_size.marked?"+":"-");
#define format_print(prop) { int n = sprintf(str_buf + offset, #prop" %03.3f, ", ppd_size.prop); str_buf[offset+n]=' '; offset += strlen(#prop) + 10; }
		format_print(width);
		format_print(length);
		format_print(left);
		format_print(bottom);
		format_print(right);
		format_print(top);
#undef format_print
		offset += sprintf(str_buf + offset, " (%2.3fx%2.3f) ", ppd_size.right - ppd_size.left, ppd_size.top - ppd_size.bottom);
		str_buf[offset] = ' ';
		sprintf(str_buf + 110, "name '%s'", ppd_size.name);
		LLog_FD("%s", str_buf);
	}
	
	ppdClose(ppd), ppd=NULL;
}

