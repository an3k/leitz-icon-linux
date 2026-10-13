CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror
.PHONY: all check clean
all: raster2printer_boundaries_recovered_test
raster2printer_boundaries_recovered_test: raster2printer_boundaries_recovered_test.c raster2printer_boundaries_recovered.c raster2printer_boundaries_recovered.h
	$(CC) $(CFLAGS) -o $@ raster2printer_boundaries_recovered_test.c raster2printer_boundaries_recovered.c
check: raster2printer_boundaries_recovered_test
	./raster2printer_boundaries_recovered_test
clean:
	rm -f raster2printer_boundaries_recovered_test
