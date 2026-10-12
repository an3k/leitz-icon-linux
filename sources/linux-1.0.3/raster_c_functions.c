// This file is subject to the Apple OS-Developed Software exception.
//
//  raster_c_functions.c
//  LeitzIcon
//
//  Created by Leonid 100500 on 10/29/13.
//  Copyright (c) 2014, Esselte IPR AB. All rights reserved.
//

#define _CURRENT_R2P_VERSION_ 

#include <stdio.h>
#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include "raster_c_functions.h"

FILE *			_fd_err = NULL;
FILE *			_fd_dump = NULL;
FILE *			_fd_stdout = NULL;

const byte ESC =  0x1B;
const byte SYN = 0x16;

int WriteData (const byte *buf, const size_t buf_size)
{
	if (0 == buf_size) {
		//	no data (it isn't a error)
		return 0;
	}
	
	if (_fd_dump) {
		//	write dump into a debug .prn file
		fwrite(buf, buf_size, 1, _fd_dump);
	}
	
	LLog("write data of size : %ld", buf_size);

	if (!_fd_stdout)
    {
		return 0;
	}
	
	const size_t written = fwrite(buf, buf_size, 1, _fd_stdout);
	if (!written)
    {
        LLog("an error occured while writing data of size : %ld", buf_size);
	}
	
	return (written != 0);
}

void FlushDataStreams ()
{
	fflush(stdout);
	if (_fd_dump) {
		fflush(_fd_dump);
	}
}

#pragma mark - DEBUG

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

char dbgprn_filename[PATH_MAX] = {0};

void OpenDebugFile ()
{
	if (_fd_dump != NULL)
    {
		LLog("%s", "debug file is open already");
		return;
	}
		
	snprintf(dbgprn_filename, sizeof(dbgprn_filename), "/Library/Caches/%s", "DEBUG");
	if (0 == mkdir(dbgprn_filename, 0755))
    {
		chmod(dbgprn_filename, 0755);
	}
	
	sprintf(dbgprn_filename + strlen(dbgprn_filename), "/dbg.prn");
	_fd_dump = fopen(dbgprn_filename, "wb");
	chmod(dbgprn_filename, 0644);
	
	if (_fd_dump)
    {
		LLog("log file: '%s'", dbgprn_filename);
	}
	else
    {
		LLog("%s", "cannot open log file");
	}
}

int SendDebugFileToPrinter ()
{
    LLog("%s", "send debug file to printer ");
	FILE * fd = fopen(dbgprn_filename, "r");
	if (fd)
    {
		int symbol;
		int count = 0;
		while (EOF != (symbol = fgetc(fd)))
        {
			if (EOF == fputc(symbol, stdout))
            {
				LLog("%s", "can not write a symbol into stdout");
				break;
			}
			++count;
		}
		fclose(fd), fd = NULL;
		LLog("%d number of bytes to stdout written", count);
	}
	else
    {
		LLog("cannot open prn file '%s'", dbgprn_filename);
	}
	return 0;
}

void CloseDebugFile ()
{
	if (_fd_dump)
    {
		fclose(_fd_dump);
		_fd_dump = NULL;
	}
}

void WriteDebugBuffer (const byte * buf, const size_t buf_size)
{
	if (_fd_dump && buf && buf_size)
    {
		fwrite(buf, buf_size, 1, _fd_dump);
	}
}

#pragma mark -

#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>

#define LLog_FD(fmt, ...) \
do { if (SHOULD_LOG) fprintf(fd, "\n%s:%d:%s(): " fmt, FILE_NAME, \
__LINE__, __func__, __VA_ARGS__); } while (0)

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
			LLog_FD("group[%d]: '%s'; num_options:%d; num_subgroups:%d", i,
					ppd->groups[i].name, ppd->groups[i].num_options, ppd->groups[i].num_subgroups);
			//	options
			for (int j=0; j<ppd->groups[i].num_options; ++j) {
				LLog_FD("option[%d]: '%s'; num_choices:%d; defchoice:'%s' ", j,
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
					LLog_FD("choice[%02d]: %s '%s' :\t '%s'", k,
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
		LLog_FD("marked %s, name '%s', width %2.3f, length %2.3f, left %2.3f, bottom %2.3f, right %2.3f, top %2.3f",
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

void test_raster()
{
	cups_raster_t *ras = cupsRasterOpen(0, CUPS_RASTER_READ);
	cups_page_header2_t header;
	int page = 0;
	int y;
	byte *buffer;
	
	while (cupsRasterReadHeader2(ras, &header))
	{
		/* setup this page */
		page ++;
		LLog("PAGE: %d %d", page, header.NumCopies);
		
		/* allocate memory for 1 line */
		buffer = (unsigned char *)malloc(header.cupsBytesPerLine);
		
		/* read raster data */
		for (y = 0; y < header.cupsHeight; y ++)
		{
			if (cupsRasterReadPixels(ras, buffer, header.cupsBytesPerLine) == 0)
            {
				break;
			}
			
			/* write raster data to printer on stdout */
		}
		
		/* finish this page */
		if (buffer) {
			free(buffer), buffer = NULL;
		}
	}
	
	cupsRasterClose(ras);
}

void PrintPageDebugInfo(cups_page_header2_t * cups_page_header)
{
	LLog("cupsBytesPerLine = %d", cups_page_header->cupsBytesPerLine);
	LLog("cupsBitsPerColor = %d", cups_page_header->cupsBitsPerColor);
	LLog("cupsBitsPerPixel = %d", cups_page_header->cupsBitsPerPixel);
	LLog("cupsColorOrder = %d", cups_page_header->cupsColorOrder);
	LLog("cupsWidth=%u; .cupsHeight = %u; .cupsBorderlessScalingFactor=%2.5f", cups_page_header->cupsWidth, cups_page_header->cupsHeight, cups_page_header->cupsBorderlessScalingFactor);
	LLog("HWResolution=[%u, %u]; .ImagingBoundingBox=[%u, %u, %u, %u]; .Margins=[%u, %u]; .PageSize=[%u, %u]; .cupsImagingBBox=[%2.5f, %2.5f, %2.5f, %2.5f]; cupsPageSize=[%2.5f, %2.5f]",
			cups_page_header->HWResolution[0], cups_page_header->HWResolution[1], cups_page_header->ImagingBoundingBox[0], cups_page_header->ImagingBoundingBox[1], cups_page_header->ImagingBoundingBox[2], cups_page_header->ImagingBoundingBox[3], cups_page_header->Margins[0], cups_page_header->Margins[1], cups_page_header->PageSize[0], cups_page_header->PageSize[1], cups_page_header->cupsImagingBBox[0], cups_page_header->cupsImagingBBox[1], cups_page_header->cupsImagingBBox[2], cups_page_header->cupsImagingBBox[3], cups_page_header->cupsPageSize[0], cups_page_header->cupsPageSize[1]);
	
	LLog("cupsPageSizeName='%s';", cups_page_header->cupsPageSizeName);
}