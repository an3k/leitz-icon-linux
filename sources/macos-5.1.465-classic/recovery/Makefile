CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror

.PHONY: all check clean
all: command2leitz_recovered_test

command2leitz_recovered_test: command2leitz_recovered_test.c command2leitz_recovered.c command2leitz_recovered.h
	$(CC) $(CFLAGS) -o $@ command2leitz_recovered_test.c command2leitz_recovered.c

check: command2leitz_recovered_test
	./command2leitz_recovered_test

clean:
	rm -f command2leitz_recovered_test command2leitz_recovered_cups

# CUPS 2.x command adapter: macOS uses its own official SDK.
# The Linux target is strictly an offline experiment with the local libcups.
ifeq ($(shell uname -s),Darwin)
CUPS_LINK = -lcups
else
CUPS_LINK = -l:libcups.so.2
endif
command2leitz_recovered_cups: command2leitz_cups.c command2leitz_recovered.c command2leitz_recovered.h
	$(CC) $(CFLAGS) -o $@ command2leitz_cups.c command2leitz_recovered.c $(CUPS_LINK)
