CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror
.PHONY: all check clean
all: original_test_raster_test original_page_debug_test test_original_output_cups_info
# The tests use mock CUPS headers and APIs. Production sources are compiled
# separately against Apple's actual CUPS SDK without this fixture.
original_test_raster_test: original_test_raster.c original_test_raster_test.c fixture/cups/raster.h
	$(CC) $(CFLAGS) -Ifixture -o $@ original_test_raster.c original_test_raster_test.c
original_page_debug_test: original_page_debug.c original_page_debug_test.c fixture/cups/raster.h
	$(CC) $(CFLAGS) -Ifixture -o $@ original_page_debug.c original_page_debug_test.c
test_original_output_cups_info: original_output_cups_info.c test_original_output_cups_info.c ppd-fixture/cups/ppd.h
	$(CC) $(CFLAGS) -Wno-format-zero-length -Ippd-fixture -o $@ original_output_cups_info.c test_original_output_cups_info.c
check: all
	./original_test_raster_test
	./original_page_debug_test
	./test_original_output_cups_info
clean:
	rm -f original_test_raster_test original_page_debug_test test_original_output_cups_info
