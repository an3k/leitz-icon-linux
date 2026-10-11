CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11
LDLIBS = -lm -ldl
SRC := driver/src
MOD := driver/modules
PROTOCOL_SOURCES := $(wildcard $(SRC)/leitz_protocol_current.c $(SRC)/leitz_protocol_native.c)
MODULE_TARGETS := $(patsubst $(SRC)/leitz_protocol_current.c,$(MOD)/leitz-current.so,$(filter %leitz_protocol_current.c,$(PROTOCOL_SOURCES))) \
                  $(patsubst $(SRC)/leitz_protocol_native.c,$(MOD)/leitz-native.so,$(filter %leitz_protocol_native.c,$(PROTOCOL_SOURCES)))
CORE_SOURCES := $(SRC)/leitz_jobs.c $(SRC)/leitz_protocols.c
CORE_HEADERS := $(SRC)/leitz_protocols.h $(SRC)/leitz_jobs.h

.PHONY: all modules cups ppd generate-media probe clean
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
ppd:
	python3 driver/build/generate_media.py --check
generate-media:
	python3 driver/build/generate_media.py
probe: leitz-probe
leitz-probe: $(SRC)/leitz_probe.c $(SRC)/leitz_probe_io.c $(SRC)/leitz_probe_io.h $(SRC)/leitz_probe_format.c $(SRC)/leitz_probe_format.h $(SRC)/leitz_status.c $(SRC)/leitz_status.h
	$(CC) $(CFLAGS) -o $@ $(SRC)/leitz_probe.c $(SRC)/leitz_probe_io.c $(SRC)/leitz_probe_format.c $(SRC)/leitz_status.c
clean:
	rm -f leitz-cli leitz-rastertoicon leitz-probe $(MOD)/*.so
