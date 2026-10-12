# Vendor Linux 1.0.3, built from its original C files.
# Requires CUPS 2.x development headers and libraries.
CC ?= cc
CFLAGS ?= -O2 -std=gnu99
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?= -lcups -lcupsimage

.PHONY: all clean
all: raster2printer

raster2printer: raster2printer.c raster_c_functions.c raster2printer.h raster_c_functions.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ raster_c_functions.c raster2printer.c $(LDLIBS)

clean:
	rm -f raster2printer
