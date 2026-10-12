CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror

.PHONY: all check clean
all: libmac_classic_command_core.a

mac_classic_command_core.o: mac_classic_command_core.c mac_classic_command_core.h
	$(CC) $(CFLAGS) -c -o $@ mac_classic_command_core.c

libmac_classic_command_core.a: mac_classic_command_core.o
	ar rcs $@ $^

smoke: smoke.c mac_classic_command_core.c mac_classic_command_core.h
	$(CC) $(CFLAGS) -o $@ smoke.c mac_classic_command_core.c

check: smoke
	./smoke

clean:
	rm -f smoke *.o *.a
