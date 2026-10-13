CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror
.PHONY: all check clean
all: original_test_raster_test original_page_debug_test
# The tests use mock CUPS headers and APIs. Production sources are compiled
# separately against Apple's actual CUPS SDK without this fixture.
original_test_raster_test: original_test_raster.c original_test_raster_test.c fixture/cups/raster.h
	$(CC) $(CFLAGS) -Ifixture -o $@ original_test_raster.c original_test_raster_test.c
original_page_debug_test: original_page_debug.c original_page_debug_test.c fixture/cups/raster.h
	$(CC) $(CFLAGS) -Ifixture -o $@ original_page_debug.c original_page_debug_test.c
check: all
	./original_test_raster_test
	./original_page_debug_test
clean:
	rm -f original_test_raster_test original_page_debug_test
