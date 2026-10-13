CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror
.PHONY: all check clean
all: raster2printer_boundaries_recovered_test raster2printer_quality_recovered_test raster2printer_dither_recovered_test classic_read_page_test classic_write_data_test
raster2printer_boundaries_recovered_test: raster2printer_boundaries_recovered_test.c raster2printer_boundaries_recovered.c raster2printer_boundaries_recovered.h
	$(CC) $(CFLAGS) -o $@ raster2printer_boundaries_recovered_test.c raster2printer_boundaries_recovered.c
raster2printer_quality_recovered_test: raster2printer_quality_recovered_test.c raster2printer_quality_recovered.c raster2printer_matrices_recovered.c raster2printer_quality_recovered.h
	$(CC) $(CFLAGS) -o $@ raster2printer_quality_recovered_test.c raster2printer_quality_recovered.c raster2printer_matrices_recovered.c
raster2printer_dither_recovered_test: raster2printer_dither_recovered_test.c raster2printer_dither_recovered.c raster2printer_matrices_recovered.c raster2printer_quality_recovered.c raster2printer_dither_recovered.h raster2printer_quality_recovered.h
	$(CC) $(CFLAGS) -o $@ raster2printer_dither_recovered_test.c raster2printer_dither_recovered.c raster2printer_matrices_recovered.c raster2printer_quality_recovered.c
classic_read_page_test: classic_read_page_test.c classic_read_page.c classic_read_page.h
	$(CC) $(CFLAGS) -o $@ classic_read_page_test.c classic_read_page.c
classic_write_data_test: classic_write_data_test.c classic_write_data.c classic_write_data.h
	$(CC) $(CFLAGS) -o $@ classic_write_data_test.c classic_write_data.c
check: raster2printer_boundaries_recovered_test raster2printer_quality_recovered_test raster2printer_dither_recovered_test classic_read_page_test classic_write_data_test
	./raster2printer_boundaries_recovered_test
	./raster2printer_quality_recovered_test
	./raster2printer_dither_recovered_test
	./classic_read_page_test
	./classic_write_data_test
clean:
	rm -f raster2printer_boundaries_recovered_test raster2printer_quality_recovered_test raster2printer_dither_recovered_test classic_read_page_test classic_write_data_test
