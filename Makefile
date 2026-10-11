CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11
LDLIBS = -lm -ldl
SRC := driver/src
CORE_SOURCES := $(SRC)/leitz_jobs.c $(SRC)/leitz_protocols.c

.PHONY: verify mock-filter media-check native-reference-test job-geometry-test protocol-isolation-test status-test probe-test integration

verify: mock-filter media-check native-reference-test job-geometry-test protocol-isolation-test status-test probe-test integration

mock-filter:
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -Itests/mocks/shim -o leitz-rastertoicon-mock $(SRC)/leitz_raster.c $(CORE_SOURCES) tests/mocks/mock_cups.c $(LDLIBS)

media-check:
	python3 tests/integration/check_media_consistency.py
	python3 driver/build/generate_media.py --check

native-reference-test:
	$(CC) $(CFLAGS) -o /tmp/leitz-native-reference-test tests/unit/test_native_reference.c $(SRC)/leitz_protocols.c $(LDLIBS)
	/tmp/leitz-native-reference-test
	$(CC) $(CFLAGS) -o /tmp/leitz-native-validate-test tests/unit/test_native_validate.c $(SRC)/leitz_protocols.c $(LDLIBS)
	/tmp/leitz-native-validate-test
	python3 tests/integration/test_native_cli.py

job-geometry-test:
	$(CC) $(CFLAGS) -o /tmp/leitz-job-geometry-test tests/unit/test_job_geometry.c $(CORE_SOURCES) $(LDLIBS)
	/tmp/leitz-job-geometry-test

protocol-isolation-test:
	python3 tests/integration/test_protocol_isolation.py

status-test:
	$(CC) $(CFLAGS) -o /tmp/leitz-status-decode-test $(SRC)/leitz_status.c tests/unit/test_status_decode.c
	/tmp/leitz-status-decode-test
	python3 tests/integration/test_status_cli.py

probe-test:
	$(CC) $(CFLAGS) -I$(SRC) -o /tmp/leitz-probe-io-test $(SRC)/leitz_probe_io.c tests/unit/test_probe_io.c
	/tmp/leitz-probe-io-test
	$(CC) $(CFLAGS) -I$(SRC) -o /tmp/leitz-probe-format-test $(SRC)/leitz_probe_format.c $(SRC)/leitz_status.c tests/unit/test_probe_format.c
	/tmp/leitz-probe-format-test
	python3 tests/integration/test_probe_cli.py

integration:
	python3 tests/integration/test_core.py
	python3 tests/integration/test_v04.py
	python3 tests/integration/test_cups_command.py
