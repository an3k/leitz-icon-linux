CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11
LDLIBS = -lm -ldl
SRC := driver/src
MOD := driver/modules

# Both modules are built independently. Removing an installed .so never
# requires rebuilding the common filter or the immutable CUPS PPD.
PROTOCOL_SOURCES := $(wildcard $(SRC)/leitz_protocol_current.c $(SRC)/leitz_protocol_native.c)
MODULE_TARGETS := $(patsubst $(SRC)/leitz_protocol_current.c,$(MOD)/leitz-current.so,$(filter %leitz_protocol_current.c,$(PROTOCOL_SOURCES))) \
                  $(patsubst $(SRC)/leitz_protocol_native.c,$(MOD)/leitz-native.so,$(filter %leitz_protocol_native.c,$(PROTOCOL_SOURCES)))
CORE_SOURCES := $(SRC)/leitz_jobs.c $(SRC)/leitz_protocols.c
CORE_HEADERS := $(SRC)/leitz_protocols.h $(SRC)/leitz_jobs.h

.PHONY: all cups modules clean mock-filter native-reference-test media-check status-test probe-test probe job-geometry-test ppd generate-media protocol-isolation-test
all: modules leitz-cli
modules: $(MODULE_TARGETS)
$(MOD)/leitz-current.so: $(SRC)/leitz_protocol_current.c $(SRC)/leitz_protocols.h
	mkdir -p $(MOD)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $(SRC)/leitz_protocol_current.c -lm
$(MOD)/leitz-native.so: $(SRC)/leitz_protocol_native.c $(SRC)/leitz_protocols.h
	mkdir -p $(MOD)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $(SRC)/leitz_protocol_native.c -lm

leitz-cli: $(SRC)/leitz_cli.c $(SRC)/leitz_status.c $(SRC)/leitz_status.h $(CORE_SOURCES) $(CORE_HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SRC)/leitz_cli.c $(SRC)/leitz_status.c $(CORE_SOURCES) $(LDLIBS)
cups: leitz-rastertoicon
leitz-rastertoicon: modules $(SRC)/leitz_raster.c $(SRC)/leitz_media.h $(CORE_SOURCES) $(CORE_HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SRC)/leitz_raster.c $(CORE_SOURCES) -lcups $(LDLIBS)
mock-filter: leitz-rastertoicon-mock
leitz-rastertoicon-mock: modules $(SRC)/leitz_raster.c tests/mocks/mock_cups.c $(SRC)/leitz_media.h $(CORE_SOURCES) $(CORE_HEADERS)
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -Itests/mocks/shim -o $@ $(SRC)/leitz_raster.c $(CORE_SOURCES) tests/mocks/mock_cups.c $(LDLIBS)
ppd:
	python3 driver/build/generate_media.py --check
generate-media:
	python3 driver/build/generate_media.py
clean:
	rm -f leitz-cli leitz-rastertoicon leitz-rastertoicon-mock leitz-probe $(MOD)/*.so

native-reference-test: all
	$(CC) $(CFLAGS) -o /tmp/leitz-native-reference-test tests/unit/test_native_reference.c $(SRC)/leitz_protocol_native.c $(LDLIBS)
	/tmp/leitz-native-reference-test
	$(CC) $(CFLAGS) -o /tmp/leitz-native-validate-test tests/unit/test_native_validate.c $(SRC)/leitz_protocol_native.c $(LDLIBS)
	/tmp/leitz-native-validate-test
	LEITZ_PROTOCOL_DIR=$(CURDIR)/$(MOD) python3 tests/integration/test_native_cli.py
job-geometry-test:
	$(CC) $(CFLAGS) -o /tmp/leitz-job-geometry-test tests/unit/test_job_geometry.c $(CORE_SOURCES) $(LDLIBS)
	LEITZ_PROTOCOL_DIR=$(CURDIR)/$(MOD) /tmp/leitz-job-geometry-test
media-check:
	python3 tests/integration/check_media_consistency.py
status-test: all
	$(CC) $(CFLAGS) -o /tmp/leitz-status-decode-test $(SRC)/leitz_status.c tests/unit/test_status_decode.c
	/tmp/leitz-status-decode-test
	python3 tests/integration/test_status_cli.py
probe: leitz-probe
leitz-probe: $(SRC)/leitz_probe.c $(SRC)/leitz_probe_io.c $(SRC)/leitz_probe_io.h $(SRC)/leitz_probe_format.c $(SRC)/leitz_probe_format.h $(SRC)/leitz_status.c $(SRC)/leitz_status.h
	$(CC) $(CFLAGS) -o $@ $(SRC)/leitz_probe.c $(SRC)/leitz_probe_io.c $(SRC)/leitz_probe_format.c $(SRC)/leitz_status.c
probe-test: leitz-probe
	$(CC) $(CFLAGS) -I$(SRC) -o /tmp/leitz-probe-io-test $(SRC)/leitz_probe_io.c tests/unit/test_probe_io.c
	/tmp/leitz-probe-io-test
	$(CC) $(CFLAGS) -I$(SRC) -o /tmp/leitz-probe-format-test $(SRC)/leitz_probe_format.c $(SRC)/leitz_status.c tests/unit/test_probe_format.c
	/tmp/leitz-probe-format-test
	python3 tests/integration/test_probe_cli.py
protocol-isolation-test: all mock-filter
	LEITZ_PROTOCOL_DIR=$(CURDIR)/$(MOD) python3 tests/integration/test_protocol_isolation.py
