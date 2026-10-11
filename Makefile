CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11
LDLIBS = -lm

# Omit either file to omit that protocol entirely from built binaries/PPD.
PROTOCOL_SOURCES := $(wildcard src/leitz_protocol_current.c src/leitz_protocol_native_linux_103.c)
ifeq ($(strip $(PROTOCOL_SOURCES)),)
$(error At least one protocol source file must be present)
endif
CORE_SOURCES := src/leitz_job.c src/leitz_protocol_registry.c $(PROTOCOL_SOURCES)
CORE_HEADERS := src/leitz_protocol.h src/leitz_job.h

.PHONY: all cups clean mock-filter native-reference-test media-check status-test probe-test probe job-geometry-test ppd protocol-isolation-test
all: leitz-cli
leitz-cli: src/leitz_cli.c src/leitz_status.c src/leitz_status.h $(CORE_SOURCES) $(CORE_HEADERS)
	$(CC) $(CFLAGS) -o $@ src/leitz_cli.c src/leitz_status.c $(CORE_SOURCES) $(LDLIBS)
cups: leitz-rastertoicon
leitz-rastertoicon: src/leitz_cups_filter.c src/leitz_media.h $(CORE_SOURCES) $(CORE_HEADERS)
	$(CC) $(CFLAGS) -o $@ src/leitz_cups_filter.c $(CORE_SOURCES) -lcups $(LDLIBS)
mock-filter: leitz-rastertoicon-mock
leitz-rastertoicon-mock: src/leitz_cups_filter.c tests/mock_cups.c src/leitz_media.h $(CORE_SOURCES) $(CORE_HEADERS)
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -Itests/shim -o $@ src/leitz_cups_filter.c $(CORE_SOURCES) tests/mock_cups.c $(LDLIBS)
ppd: dist/LeitzIcon.ppd
dist/LeitzIcon.ppd: cups/LeitzIcon.ppd tools/build_ppd.py Makefile $(PROTOCOL_SOURCES)
	mkdir -p dist
	python3 tools/build_ppd.py cups/LeitzIcon.ppd $@
clean:
	rm -f leitz-cli leitz-rastertoicon leitz-rastertoicon-mock leitz-probe dist/LeitzIcon.ppd

native-reference-test: leitz-cli
	@if test -f src/leitz_protocol_native_linux_103.c; then \
	  $(CC) $(CFLAGS) -o /tmp/leitz-native-reference-test tests/test_native_reference.c src/leitz_protocol_registry.c src/leitz_protocol_native_linux_103.c $(LDLIBS) && \
	  /tmp/leitz-native-reference-test && \
	  $(CC) $(CFLAGS) -o /tmp/leitz-native-validate-test tests/test_native_validate.c src/leitz_protocol_registry.c src/leitz_protocol_native_linux_103.c $(LDLIBS) && \
	  /tmp/leitz-native-validate-test && \
	  python3 tests/test_native_cli.py; \
	else echo "SKIP NativeLinux103 not linked"; fi

job-geometry-test:
	$(CC) $(CFLAGS) -o /tmp/leitz-job-geometry-test tests/test_job_geometry.c $(CORE_SOURCES) $(LDLIBS)
	/tmp/leitz-job-geometry-test
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
protocol-isolation-test:
	python3 tests/test_protocol_isolation.py
