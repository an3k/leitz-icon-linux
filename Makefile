CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11
LDLIBS = -lm
.PHONY: all cups clean test mock-filter native-reference-test
all: leitz-cli
leitz-cli: src/leitz_cli.c src/leitz_codec.c src/leitz_codec.h
	$(CC) $(CFLAGS) -o $@ src/leitz_cli.c src/leitz_codec.c $(LDLIBS)
cups: leitz-rastertoicon
leitz-rastertoicon: src/leitz_cups_filter.c src/leitz_codec.c src/leitz_codec.h src/leitz_media.h
	$(CC) $(CFLAGS) -o $@ src/leitz_cups_filter.c src/leitz_codec.c -lcups $(LDLIBS)
mock-filter: leitz-rastertoicon-mock
leitz-rastertoicon-mock: src/leitz_cups_filter.c src/leitz_codec.c tests/mock_cups.c src/leitz_media.h
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -Itests/shim -o $@ src/leitz_cups_filter.c src/leitz_codec.c tests/mock_cups.c $(LDLIBS)
clean:
	rm -f leitz-cli leitz-rastertoicon leitz-rastertoicon-mock

native-reference-test:
	$(CC) $(CFLAGS) -o /tmp/leitz-native-reference-test src/leitz_native_reference.c src/leitz_native_validate.c tests/test_native_reference.c
	/tmp/leitz-native-reference-test
	$(CC) $(CFLAGS) -o /tmp/leitz-native-validate-test src/leitz_native_reference.c src/leitz_native_validate.c tests/test_native_validate.c
	/tmp/leitz-native-validate-test
