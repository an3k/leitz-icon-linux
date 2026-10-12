// This file is subject to the Apple OS-Developed Software exception.
//
//  raster2printer.c
//  LeitzIcon
//
//  Created by Leonid 100500 on 10/29/13.
//  Copyright (c) 2014, Esselte IPR AB. All rights reserved.
//

#include "raster2printer.h"
#include <signal.h>
#include <fcntl.h>

#define DefaultLabelLines 31890 // (27000mm)7653.54345703125pt / 72 * 300 = 31890 lines

volatile int terminate_flag = 0;
static int job_size = 0;

void ProcessDbgRasterLine(unsigned char bytes_per_line);


void signal_handler(int signal)
{
	fprintf(stderr, "\nINFO: Cancelling the job...");
	terminate_flag = 1;
}

int main(int argc, char* argv[])
{
	LLog("%s", "raster filter started");
	int result = 0;
	
	terminate_flag = 0;
	signal(SIGTERM, signal_handler);
	
	//	Leonid: I hoped it would help, it didn't
	//	stdout in this case must be an open printer stream
	if (setvbuf(stdout, NULL, _IOFBF, 4096))
    {
		LLog("%s", "cannot set the stdout buffer size");
	}
    
    job_size = 0;
	//	Leonid: testing and debugging
	result = raster2printer(argc, argv);
	
    LLog(" job size : %i ", job_size);
	LLog("%s", "raster filter finished\n");
	return result;
}

#pragma mark -

void PrintErrorPage (const unsigned short lines_number)
{
	byte buf_finish[] = {
		ESC, 'f', (byte)(lines_number >> 8), (byte)lines_number,	//	skip N lines
		ESC, 'B', 0,																//	skip 0 bytes
		ESC, 'D', 4,																//	bytes per line cmd
		SYN,
		0xAA, 0x55, 0xAA, 0x55,
		SYN,
		0x55, 0xAA, 0x55, 0xAA,
	};
    
    int buf_size = sizeof(buf_finish);
    job_size += buf_size;
	WriteData(buf_finish, buf_size);
}

int raster2printer(int argc, char* argv[])
{
	print_descriptor	print_descriptor;
	bzero(&print_descriptor, sizeof(print_descriptor));
	print_descriptor.optimizationLevel = 1;
    
    int buf_size = 0;
	
	setbuf(stderr, NULL);
	
	if ((argc < 6) || (argc > 7))
	{
		LLog("%s", "error: (argc < 6) || (argc > 7)");
		return (1);
	}
	
	int fd = 0;
	if (argc == 7)
	{
		LLog("opening file from argv[6]='%s'", argv[6]);
		if ((fd = open(argv[6], O_RDONLY)) == -1)
		{
			LLog("cannot open file argv[6]='%s'", argv[6]);
			sleep(1);
			return (1);
		}
	}
	
	print_descriptor.cupsRaster = cupsRasterOpen(fd, CUPS_RASTER_READ);
	if (!print_descriptor.cupsRaster)
	{
		perror("ERROR: Unable to open raster file - ");
		return (1);
	}
	
	//InitDocumentWithOptions
	ppd_file_t* ppd = ppdOpenFile(getenv("PPD"));
	if (!ppd)
	{
		perror("WARNING: Unable to open ppd file, use default settings - ");
		return (-1);
	}
	
	cups_option_t*  cups_options = NULL;
	const int		option_count = cupsParseOptions(argv[5], 0, &cups_options);
	
    ppdMarkDefaults(ppd);
	int i = 0;
    //	// mark general options as ppd options
    for (i = 0; i < option_count; ++i)
    {
        LLog("option[%d]:'%s'='%s'", i, cups_options[i].name, cups_options[i].value);
        if (!strcmp(cups_options[i].name, "ContinuousPaperLength"))
        {
            print_descriptor.pageLength = atoi(cups_options[i].value);
            LLog("~ContinuousPaperLength value = %i",  print_descriptor.pageLength);
        }
        
        if (cups_options[i].value)
        {
            int conflicts = ppdMarkOption(ppd, cups_options[i].name, cups_options[i].value);

            if (conflicts)
            {
                LLog("got %d conflicts for option '%s'", conflicts, cups_options[i].name);
            }
        }
    }
	
	LLog("%s", "process options");
	// do CUPS specific
	cupsMarkOptions(ppd, option_count, cups_options);
	
	ppd_choice_t *
	choice = ppdFindMarkedChoice(ppd, "Resolution");
	if (choice && choice->choice)
	{
		print_descriptor.hiResolution = !strcasecmp(choice->choice, "600dpi");
	}
	
	choice = ppdFindMarkedChoice(ppd, "DebugMode");
	if (choice && choice->choice)
    {
		if (!strcmp(choice->choice, "Mesh"))
        {
			print_descriptor.debugMode = debug_mode_mesh;
		}
		else if (!strcmp(choice->choice, "Debug"))
        {
			print_descriptor.debugMode = debug_mode_on;
		}
		else if (!strcmp(choice->choice, "PrintToFile"))
        {
			print_descriptor.debugMode = debug_mode_prn;
		}
	}
	
    choice = ppdFindMarkedChoice(ppd, "PrintThroughPRN");
    if (choice && choice->choice)
    {
		print_descriptor.printThroughPRN = !strcmp(choice->choice, "ON");
	}
	
	choice = ppdFindMarkedChoice(ppd, "ContinuousMedia");
	if (choice && choice->choice)
    {
		print_descriptor.isContinuousMedia = !strcmp(choice->choice, "ON");
	}
    
	choice = ppdFindMarkedChoice(ppd, "OptimizationLevel");
	if (choice && choice->choice)
    {
		const unsigned int val = atoi(choice->choice);
		print_descriptor.optimizationLevel = val < 4 ? val : 0;
	}
    
    choice = ppdFindMarkedChoice(ppd, "LabelCutter");
	if (choice && choice->choice)
    {
		print_descriptor.cutEveryLabel = !strcmp(choice->choice, "ON");
	}
    
    choice = ppdFindMarkedChoice(ppd, "ShouldCorrectMargins");
    if (choice && choice->choice)
    {
        print_descriptor.shouldCorrectMargins = !strcmp(choice->choice, "YES");
    }
    
	cupsFreeOptions(option_count, cups_options);
	ppdClose(ppd);
	
	LLog("options: resolution:%s; continuous:%s; dbg-mod:%d; opt-lvl:%d; cut-every:%s",
			print_descriptor.hiResolution ? "hi":"low",
			print_descriptor.isContinuousMedia?"yes":"no",
			print_descriptor.debugMode,
			print_descriptor.optimizationLevel,
			print_descriptor.cutEveryLabel?"yes":"no");
    
	
	OpenDebugFile();
	int write_result = 0;
	_fd_stdout = print_descriptor.printThroughPRN != 0 ? NULL : stdout;
	
	//	start document
	const byte resolution_code = (byte)(print_descriptor.hiResolution ? 'i':'h');
	LLog("resolution_code = %c ", resolution_code);
	byte cmd_buffer[] = {
		ESC, '@',				//			reset to defaults (from the previous job)
		ESC, resolution_code	//			quality
	};
    
    buf_size = sizeof(cmd_buffer);
    job_size += buf_size;
	write_result = WriteData(cmd_buffer, buf_size);
    
	LLog("%s", "started printing pages");
	
	byte buf_form_feed[] = {
		ESC, 'G',		//	short form feed
		ESC, 'E'		//	long form feed
	};
    
	int i_page = 0;			//	current page index (beginning from 0)
	int print_result = 0;	//	0 is OK
	cups_page_header2_t cups_page_header;
    print_descriptor.lastBPLValue = print_descriptor.bytesPerLine;
	for (; (print_result == 0) && cupsRasterReadHeader2(print_descriptor.cupsRaster, &cups_page_header); ++i_page)
	{
		if (i_page)
        {
			//	finalize the previous page (skip the 1st)
            buf_size = print_descriptor.cutEveryLabel ? 4 : 2;
            job_size += buf_size;
			write_result = WriteData(buf_form_feed, buf_size);
			FlushDataStreams();
		}
		
		if (terminate_flag)
        {
			break;
		}
		
		LLog("cups_page_header, media type/name = %u/'%s';",
				cups_page_header.cupsMediaType, cups_page_header.cupsPageSizeName);
        PrintPageDebugInfo(&cups_page_header);
        //	prepare print_descriptor
		print_descriptor.pageWidthPx = cups_page_header.cupsWidth;
		print_descriptor.pageHeightPx = cups_page_header.cupsHeight;
		print_descriptor.cupsBytesPerLine = cups_page_header.cupsBytesPerLine;
		if (print_descriptor.cupsBytesPerLine > 120)
        {
			LLog("ERROR: bytes per line (%u > 120) set to 120", print_descriptor.cupsBytesPerLine);
			print_descriptor.bytesPerLine = 120;
			print_descriptor.pageWidthPx = 120*8;
			if (print_descriptor.cupsBytesPerLine > 1024)
            {
				LLog("INFO: too wide raster (%u). Full stop.", print_descriptor.cupsBytesPerLine);
				PrintErrorPage(100);
				i_page = -1;
				break;
			}
		}
		else
        {
			print_descriptor.bytesPerLine = (byte)print_descriptor.cupsBytesPerLine;
		}
        
		//	print next page
		print_result = ProcessPage(&print_descriptor, i_page);
		//	end of page
    }
    
	//	finalize the document
    buf_size = sizeof(buf_form_feed);
    job_size += buf_size;
    write_result = WriteData(buf_form_feed, buf_size);
   
	cupsRasterClose(print_descriptor.cupsRaster);
	print_descriptor.cupsRaster = NULL;
	if (fd != 0)
    {
		close(fd);
	}
	
	CloseDebugFile();
	
	if (print_descriptor.printThroughPRN != 0)
    {
		//	here the data stream from a .prn goes to a printer
		SendDebugFileToPrinter ();
        LLog("%s", "print through PRN file ");
	}
    else
    {
        LLog("%s", "do not print through PRN file ");
    }
	
	return print_result;
}

int ProcessPage(print_descriptor *print_descriptor, const int i_page)
{
	LLog("PAGE #%d; (bpl/w-px/h-px)=(%d/%d/%d)",
			i_page, print_descriptor->bytesPerLine, print_descriptor->pageWidthPx, print_descriptor->pageHeightPx);
	
	int result = 0;
	int write_result = 0;
    int buf_size = 0;
	
	const unsigned bytes_per_line = print_descriptor->bytesPerLine;
	
	//	start page
	const unsigned short prn_v_margins = print_descriptor->hiResolution ? 60 : 30;
	unsigned short print_length = print_descriptor->isContinuousMedia ? print_descriptor->pageLength : print_descriptor->pageHeightPx;
        
    if (print_descriptor->hiResolution && !print_descriptor->isContinuousMedia)
    {
        print_length /= 2;
    }
	const unsigned short page_width = print_descriptor->pageWidthPx;
    byte buf_start[] = {
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
        ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC,
		ESC, 'L', (byte)(print_length >> 8), (byte)print_length,	//	set label length
		ESC, 'w', (byte)(page_width >> 8), (byte)page_width,		//	set print width
		ESC, 'B', 0,																//	skip 0 bytes (it makes 'w' work)
	};
    buf_size = sizeof(buf_start);
    job_size += buf_size;
	write_result = WriteData(buf_start, buf_size);
	unsigned short skipb_last_value = 0;	//	for <ESC B> cmd
	
	//	allocate & prepare a pixel buffer
	unsigned char prefix[] = {
        ESC, 'D', (byte)print_descriptor->bytesPerLine,
		SYN
	};						//	bitmap cmd
	const size_t prefix_size = sizeof(prefix);
	const size_t pixel_buffer_sz = print_descriptor->cupsBytesPerLine + prefix_size;
	//	pixel_buffer_printable_sz maybe smaller than pixel_buffer_sz
	const size_t pixel_buffer_printable_sz = bytes_per_line + prefix_size;
	byte * pixel_buffer = (byte *)malloc(pixel_buffer_sz);
	if (!pixel_buffer)
    {
		fprintf(stderr, "\nERROR: cannot allocate memory for a pixel buffer");
		return -1;
	}
	unsigned char *pixel_buffer_data = pixel_buffer + prefix_size;
	memcpy(pixel_buffer, prefix, prefix_size);
	
	unsigned short lines_to_skip = 0;
	int count_skipped_lines = 0;
    int continuousLabelBitmapStartLine = 0;
    int continuousLabelBitmapFinishLine = print_descriptor->pageHeightPx;
    int multiplier = print_descriptor->hiResolution ? 2 : 1;
    
    LLog("1) Start Line = %d, Finish Line = %d Default lines count : %d ,  All lines  = %d ", continuousLabelBitmapStartLine, continuousLabelBitmapFinishLine, DefaultLabelLines, print_length);
    if (print_descriptor->isContinuousMedia)
    {
        if (print_descriptor->shouldCorrectMargins)
        {
            continuousLabelBitmapStartLine += prn_v_margins;
        }
        continuousLabelBitmapFinishLine -=  multiplier * (DefaultLabelLines - print_descriptor->pageLength);
    }
    
    LLog("2) Start Line = %d, Finish Line = %d Default lines count : %d ,  All lines  = %d ", continuousLabelBitmapStartLine, continuousLabelBitmapFinishLine, DefaultLabelLines, print_length);
	//	process all the raster lines
	for (size_t y_line = 0; y_line < continuousLabelBitmapFinishLine; ++y_line)
	{
		if (!(y_line & 0x1F))
        {
			//	show progress info
			int progress_value = (int)(100.0 * (float)y_line / (float)print_descriptor->pageHeightPx);
			fprintf(stderr, "\nINFO: Printing page %d, %d%% complete...", i_page + 1, progress_value);
            fprintf(stderr, "\nATTR: job-media-progress=%d",progress_value);
		}
		
		//	copy bitmap into the buffer after the prefix
		size_t bytesRead = cupsRasterReadPixels(print_descriptor->cupsRaster, pixel_buffer_data, print_descriptor->cupsBytesPerLine);
        
        if (bytesRead != print_descriptor->cupsBytesPerLine)
        {
			LLog("cupsRasterReadPixels() failed: expected %u, read %lu", bytes_per_line, bytesRead);
			PrintErrorPage(100);
			//	get all the data from the cups raster (I'm not sure I need it)
			for (; y_line < print_descriptor->pageHeightPx; ++y_line)
            {
				cupsRasterReadPixels(print_descriptor->cupsRaster, pixel_buffer_data, print_descriptor->cupsBytesPerLine);
            }
            result = 0;
            break;
        }
		
		if (print_descriptor->debugMode == debug_mode_mesh)
        {
			//	Leonid: mark the area
			pixel_buffer_data[0] |= 0x80;
			pixel_buffer_data[bytes_per_line/2] |= 0x10;
			pixel_buffer_data[bytes_per_line-1] |= 0x01;
			if (0 == y_line || print_descriptor->pageHeightPx-1 == y_line)
            {
				memset(pixel_buffer_data, 0xFF, bytes_per_line);
			}
			if (y_line == (print_descriptor->pageHeightPx / 2))
            {
				memset(pixel_buffer_data, 0xFF, bytes_per_line);
			}
		}

        if ((print_descriptor->isContinuousMedia && y_line > continuousLabelBitmapStartLine) || !print_descriptor->isContinuousMedia)
        {
            //	optimization, reduce the line's number
            if (0 != print_descriptor->optimizationLevel)
            {
                unsigned short whites_before = 0;
                for (; whites_before < bytes_per_line; ++whites_before)
                {
                    if (pixel_buffer_data[whites_before])
                    {
                        break;
                    }
                }
                if (whites_before == bytes_per_line)
                {
                    ++lines_to_skip;
                    ++count_skipped_lines;
                }
                else
                {
                    //	skip <lines_to_skip> lines and then print the current one
                    if (lines_to_skip)
                    {
                        LLog("skip lines: %d",
                                (int)lines_to_skip);
                        
                        unsigned char buf_skip[] =
                        {
                            ESC, 'f', (unsigned char)(lines_to_skip >> 8), (unsigned char)lines_to_skip
                        };
                        buf_size = sizeof(buf_skip);
                        job_size += buf_size;
                        write_result = WriteData(buf_skip, buf_size);
                        lines_to_skip = 0;
                    }
                    
                    if (2 == print_descriptor->optimizationLevel)
                    {
                        //	process a line as a set of white & black bytes
                        const unsigned short MinToSkip = 0;
                        if (whites_before < MinToSkip)
                        {
                            whites_before = 0;
                        }
                        unsigned short whitesAfter = 0;
                        
                        if (whites_before)
                        {
                            for (unsigned short i = bytes_per_line-1; i > whites_before; --i, ++whitesAfter)
                            {
                                if (pixel_buffer_data[i])
                                {
                                    break;
                                }
                            }
                        }
                        
                        if (whitesAfter < MinToSkip)
                        {
                            whitesAfter = 0;
                        }
                        //	skip whites before cmd
                        if (skipb_last_value != whites_before)
                        {
                            unsigned char buf_bskip[] = {
                                ESC, 'B', (unsigned char)whites_before,
                            };
                            
                            buf_size = sizeof(buf_bskip);
                            job_size += buf_size;
                            write_result = WriteData(buf_bskip, buf_size);
                            skipb_last_value = whites_before;
                        }
                        
                        //	update BPL if changed
                        const unsigned blacks = bytes_per_line - whites_before - whitesAfter;
                        if (((unsigned)print_descriptor->lastBPLValue != blacks) || (print_descriptor->optimizationLevel == 3 ))
                        {
                            print_descriptor->lastBPLValue = (unsigned char)blacks;
                            unsigned char buf_bpl[] = {
                                ESC, 'D', print_descriptor->lastBPLValue
                            };
                            buf_size = sizeof(buf_bpl);
                            job_size += buf_size;
                            write_result = WriteData(buf_bpl, buf_size);
                        }
                        
                        //	insert SYN before black bytes
                        *(pixel_buffer_data + whites_before - 1) = SYN;
                        buf_size = blacks + 1;
                        job_size += buf_size;
                        write_result = WriteData(pixel_buffer_data + whites_before - 1, buf_size);
                        
                        LLog("skip : white/black/bpl=(%d/%d/%d)",
                                (int)whites_before, (int)blacks, (int)bytes_per_line);
                    }
                    else
                    {
                        job_size += pixel_buffer_printable_sz;
                        write_result = WriteData(pixel_buffer, pixel_buffer_printable_sz);
                    }
                }
            }
            else
            {
                job_size += pixel_buffer_printable_sz;
                write_result = WriteData(pixel_buffer, pixel_buffer_printable_sz);
            }	//	end of optimization
        }
		
		if (terminate_flag)
        {
			break;
		}
	}
	
	//	end page
	LLog("end page func (skipped %d lines out of %d)", count_skipped_lines, print_length);
	fprintf(stderr, "\nINFO: Page %d completed.", i_page+1);
    fprintf(stderr, "\nATTR: job-media-progress=100");
    
	free(pixel_buffer);
	
	return result;
}
