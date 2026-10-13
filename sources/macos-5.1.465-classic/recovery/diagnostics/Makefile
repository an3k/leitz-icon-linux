CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror
.PHONY: all check clean
all: original_test_raster_test
# The test uses a fake CUPS header and fake functions; the production source
# itself is not changed and is compiled against real headers on macOS.
original_test_raster_test: original_test_raster.c original_test_raster_test.c fixture/cups/raster.h
	$(CC) $(CFLAGS) -Ifixture -o $@ original_test_raster.c original_test_raster_test.c
check: original_test_raster_test
	./original_test_raster_test
clean:
	rm -f original_test_raster_test
