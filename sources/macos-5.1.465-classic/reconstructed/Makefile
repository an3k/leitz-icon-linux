CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror
LIB_SRCS = mac_classic_command_core.c mac_classic_raster_boundaries.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

.PHONY: all check clean
all: libmac_classic_fragments.a

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

libmac_classic_fragments.a: $(LIB_OBJS)
	ar rcs $@ $^

smoke: smoke.c $(LIB_SRCS) mac_classic_command_core.h mac_classic_raster_boundaries.h
	$(CC) $(CFLAGS) -o $@ smoke.c $(LIB_SRCS)

check: smoke
	./smoke

clean:
	rm -f smoke *.o *.a
