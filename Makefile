CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11
LDLIBS = -lm
.PHONY: all cups clean media-check
all: leitz-cli
leitz-cli: src/leitz_cli.c src/leitz_codec.c src/leitz_codec.h
	$(CC) $(CFLAGS) -o $@ src/leitz_cli.c src/leitz_codec.c $(LDLIBS)
cups: leitz-rastertoicon
leitz-rastertoicon: src/leitz_cups_filter.c src/leitz_codec.c src/leitz_codec.h src/leitz_media.h
	$(CC) $(CFLAGS) -o $@ src/leitz_cups_filter.c src/leitz_codec.c -lcups $(LDLIBS)
media-check:
	python3 build/check_media.py
clean:
	rm -f leitz-cli leitz-rastertoicon leitz-rastertoicon-mock
