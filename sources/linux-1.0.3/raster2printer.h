//
//  raster2printer.h
//  LeitzIcon
//
//  Created by Leonid 100500 on 10/29/13.
//  Copyright (c) 2014, Esselte IPR AB. All rights reserved.
//

#ifndef SampleRaster_raster2printer_h
#define SampleRaster_raster2printer_h

#include <cups/cups.h>
#include <cups/raster.h>
#include "raster_c_functions.h"

#  ifdef __cplusplus
extern "C" {
#  endif
	

	int raster2printer(int argc, char* argv[]);
	int ProcessPage(print_descriptor * printDescriptor, const int i_page);
	void PrintPageDebugInfo(cups_page_header2_t * cups_page_header);

#  ifdef __cplusplus
	};
#  endif


#endif
