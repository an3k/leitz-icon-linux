CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11
LDLIBS = -lm
.PHONY: all cups clean test mock-filter native-reference-test media-check status-test probe-test probe
all: leitz-cli
leitz-cli: src/leitz_cli.c src/leitz_codec.c src/leitz_codec.h src/leitz_status.c src/leitz_status.h src/leitz_native_validate.c src/leitz_native_job_validate.c src/leitz_native_reference.h
	$(CC) $(CFLAGS) -o $@ src/leitz_cli.c src/leitz_codec.c src/leitz_status.c src/leitz_native_validate.c src/leitz_native_job_validate.c $(LDLIBS)
cups: leitz-rastertoicon
leitz-rastertoicon: src/leitz_cups_filter.c src/leitz_codec.c src/leitz_codec.h src/leitz_media.h src/leitz_native_reference.c src/leitz_native_reference.h src/leitz_native_validate.c src/leitz_native_job_validate.c
	$(CC) $(CFLAGS) -o $@ src/leitz_cups_filter.c src/leitz_codec.c src/leitz_native_reference.c src/leitz_native_validate.c src/leitz_native_job_validate.c -lcups $(LDLIBS)
mock-filter: leitz-rastertoicon-mock
leitz-rastertoicon-mock: src/leitz_cups_filter.c src/leitz_codec.c src/leitz_native_reference.c src/leitz_native_validate.c src/leitz_native_job_validate.c tests/mock_cups.c src/leitz_media.h
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -Itests/shim -o $@ src/leitz_cups_filter.c src/leitz_codec.c src/leitz_native_reference.c src/leitz_native_validate.c src/leitz_native_job_validate.c tests/mock_cups.c $(LDLIBS)
clean:
	rm -f leitz-cli leitz-rastertoicon leitz-rastertoicon-mock leitz-probe

native-reference-test:
	$(CC) $(CFLAGS) -o /tmp/leitz-native-reference-test src/leitz_native_reference.c src/leitz_native_validate.c tests/test_native_reference.c
	/tmp/leitz-native-reference-test
	$(CC) $(CFLAGS) -o /tmp/leitz-native-validate-test src/leitz_native_reference.c src/leitz_native_validate.c tests/test_native_validate.c
	/tmp/leitz-native-validate-test
	$(MAKE) leitz-cli
	python3 tests/test_native_cli.py

media-check:
	python3 tools/check_media_consistency.py

status-test: leitz-cli
	$(CC) $(CFLAGS) -o /tmp/leitz-status-decode-test src/leitz_status.c tests/test_status_decode.c
	/tmp/leitz-status-decode-test
	python3 tests/test_status_cli.py

probe: leitz-probe
leitz-probe: src/leitz_probe.c src/leitz_probe_io.c src/leitz_probe_io.h src/leitz_probe_format.c src/leitz_probe_format.h src/leitz_status.c src/leitz_status.h
	$(CC) $(CFLAGS) -o $@ src/leitz_probe.c src/leitz_probe_io.c src/leitz_probe_format.c src/leitz_status.c

probe-test: leitz-probe
	$(CC) $(CFLAGS) -Isrc -o /tmp/leitz-probe-io-test src/leitz_probe_io.c tests/test_probe_io.c
	/tmp/leitz-probe-io-test
	$(CC) $(CFLAGS) -Isrc -o /tmp/leitz-probe-format-test src/leitz_probe_format.c src/leitz_status.c tests/test_probe_format.c
	/tmp/leitz-probe-format-test
	python3 tests/test_probe_cli.py
