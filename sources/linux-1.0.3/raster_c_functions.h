//
//  raster_c_functions.h
//  LeitzIcon
//
//  Created by Leonid 100500 on 10/29/13.
//  Copyright (c) 2014, Esselte IPR AB. All rights reserved.
//

#ifndef SampleRaster_raster_c_functions_h
#define SampleRaster_raster_c_functions_h

#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <string.h>

#define SHOULD_LOG 1
#define FILE_NAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LLog(fmt, ...) \
do { if (SHOULD_LOG) fprintf(stderr, "\n%s:%d:%s(): " fmt, FILE_NAME, \
__LINE__, __func__, __VA_ARGS__); } while (0)

#  ifdef __cplusplus
extern "C" {
#  endif
    
#ifndef byte
#define byte unsigned char
#endif
    
	typedef enum debug_mode {
		debug_mode_off = 0,
		debug_mode_on = 1,
		debug_mode_mesh = 2,
		debug_mode_prn = 3,		//	write raw data into .prn file
	} debug_mode;
	
	typedef struct print_descriptor
	{
		cups_raster_t * cupsRaster;
		int				isContinuousMedia;		//	0 = die cut; !0 = continuous;
        int             shouldCorrectMargins;
        int				printThroughPRN;		//	0 = do not print through PRN; !0 = print through PRN;
        int				cutEveryLabel;			//	!0= cut every single page;
		int				hiResolution;			//	0 = low resolution
        int				pageLength;             // continuous label length
		debug_mode		debugMode;
		unsigned short	pageHeightPx;			//	page width and height in pixels
		unsigned short	pageWidthPx;
		byte			bytesPerLine;			//	printable bytes per line, 120 max
		byte			lastBPLValue;			//	bytes per line value for ESC D command
		unsigned		cupsBytesPerLine;		//	bytes per line from cups, may exceed 120
		int				optimizationLevel;		//	0/1/2 0-
	} print_descriptor;
    
	//	debug's .prn file descriptor
	extern	FILE *			_fd_dump;
	extern	FILE *			_fd_stdout;
	
	extern	const byte ESC;
	extern	const byte SYN;
    
    int WriteData(const byte *buf, const size_t buf_size);
    
    void OpenDebugFile ();
    void CloseDebugFile ();
    void WriteDebugBuffer (const byte * buf, const size_t buf_size);
	void FlushDataStreams ();
    int SendDebugFileToPrinter ();

	
	//	prints all info available in the .ppd file
	//	1 path to a .ppd file\
	//	2 output stream (you may use stderr)
	void output_cups_info(const char *cups_file, FILE * fd);
	void test_raster();
	
	
#  ifdef __cplusplus
};
#  endif

#endif