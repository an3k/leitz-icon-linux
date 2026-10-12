CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror
LIB_SRCS = mac_classic_command_core.c mac_classic_raster_boundaries.c mac_classic_dither.c mac_classic_process_page.c mac_classic_read_page.c mac_classic_job.c mac_classic_output.c mac_classic_status_exchange.c mac_classic_command_dispatch.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

.PHONY: all check clean
all: libmac_classic_fragments.a

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

libmac_classic_fragments.a: $(LIB_OBJS)
	ar rcs $@ $^

smoke: smoke.c $(LIB_SRCS) mac_classic_command_core.h mac_classic_raster_boundaries.h mac_classic_dither.h mac_classic_process_page.h mac_classic_read_page.h mac_classic_job.h
	$(CC) $(CFLAGS) -o $@ smoke.c $(LIB_SRCS)

job_smoke: job_smoke.c $(LIB_SRCS) mac_classic_job.h
	$(CC) $(CFLAGS) -o $@ job_smoke.c $(LIB_SRCS)

io_smoke: io_smoke.c mac_classic_output.c mac_classic_status_exchange.c mac_classic_command_core.c mac_classic_command_dispatch.c
	$(CC) $(CFLAGS) -o $@ io_smoke.c mac_classic_output.c mac_classic_status_exchange.c mac_classic_command_core.c mac_classic_command_dispatch.c

check: smoke job_smoke io_smoke
	./smoke
	./job_smoke
	./io_smoke

clean:
	rm -f smoke job_smoke io_smoke mac_classic_cups_filter *.o *.a

# Optional: real CUPS 2.x development headers + libraries required.
# This is an unqualified reconstructed filter: DO NOT install on a printer.
mac_classic_cups_filter: mac_classic_cups_filter.c $(LIB_SRCS)
	$(CC) $(CFLAGS) -o $@ mac_classic_cups_filter.c $(LIB_SRCS) -lcups -lcupsimage
