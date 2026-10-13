# Linux 1.0.3 original-source build helper (project-authored).
# Keeps the unmodified manufacturer C and PPD under ../../originals/.
# Run: make -C tooling/linux-1.0.3
CC ?= cc
CFLAGS ?= -O2 -std=gnu99
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?= -lcups -lcupsimage
ORIGINAL := ../../originals/linux-1.0.3
SOURCE := $(ORIGINAL)/raster2printer.c $(ORIGINAL)/raster_c_functions.c
HEADERS := $(ORIGINAL)/raster2printer.h $(ORIGINAL)/raster_c_functions.h

.PHONY: all clean
all: raster2printer

raster2printer: $(SOURCE) $(HEADERS)
	$(CC) $(CPPFLAGS) -I$(ORIGINAL) $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCE) $(LDLIBS)

clean:
	rm -f raster2printer
