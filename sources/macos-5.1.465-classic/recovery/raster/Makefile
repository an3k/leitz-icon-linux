CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror
.PHONY: all check clean
all: raster2printer_boundaries_recovered_test raster2printer_quality_recovered_test raster2printer_dither_recovered_test classic_read_page_test classic_write_data_test classic_process_page_modes_01_test classic_process_page_modes_23_test classic_raster_job_recovered_test original_prn_helpers_test original_log_helpers_test original_status_decoder_test original_status_gate_test
raster2printer_boundaries_recovered_test: raster2printer_boundaries_recovered_test.c raster2printer_boundaries_recovered.c raster2printer_boundaries_recovered.h
	$(CC) $(CFLAGS) -o $@ raster2printer_boundaries_recovered_test.c raster2printer_boundaries_recovered.c
raster2printer_quality_recovered_test: raster2printer_quality_recovered_test.c raster2printer_quality_recovered.c raster2printer_matrices_recovered.c raster2printer_quality_recovered.h
	$(CC) $(CFLAGS) -o $@ raster2printer_quality_recovered_test.c raster2printer_quality_recovered.c raster2printer_matrices_recovered.c
raster2printer_dither_recovered_test: raster2printer_dither_recovered_test.c raster2printer_dither_recovered.c raster2printer_matrices_recovered.c raster2printer_quality_recovered.c raster2printer_dither_recovered.h raster2printer_quality_recovered.h
	$(CC) $(CFLAGS) -o $@ raster2printer_dither_recovered_test.c raster2printer_dither_recovered.c raster2printer_matrices_recovered.c raster2printer_quality_recovered.c
classic_read_page_test: classic_read_page_test.c classic_read_page.c classic_read_page.h
	$(CC) $(CFLAGS) -o $@ classic_read_page_test.c classic_read_page.c
classic_write_data_test: classic_write_data_test.c classic_write_data.c classic_write_data.h
	$(CC) $(CFLAGS) -o $@ classic_write_data_test.c classic_write_data.c
classic_process_page_modes_01_test: classic_process_page_modes_01_test.c classic_process_page_modes_01.c classic_process_page_modes_01.h
	$(CC) $(CFLAGS) -o $@ classic_process_page_modes_01_test.c classic_process_page_modes_01.c
classic_process_page_modes_23_test: classic_process_page_modes_23_test.c classic_process_page_modes_23.c classic_process_page_modes_23.h
	$(CC) $(CFLAGS) -o $@ classic_process_page_modes_23_test.c classic_process_page_modes_23.c
classic_raster_job_recovered_test: classic_raster_job_recovered_test.c classic_raster_job_recovered.c classic_read_page.c classic_process_page_modes_01.c classic_process_page_modes_23.c raster2printer_boundaries_recovered.c raster2printer_dither_recovered.c raster2printer_quality_recovered.c raster2printer_matrices_recovered.c
	$(CC) $(CFLAGS) -o $@ classic_raster_job_recovered_test.c classic_raster_job_recovered.c classic_read_page.c classic_process_page_modes_01.c classic_process_page_modes_23.c raster2printer_boundaries_recovered.c raster2printer_dither_recovered.c raster2printer_quality_recovered.c raster2printer_matrices_recovered.c
original_prn_helpers_test: original_prn_helpers_test.c original_prn_helpers.c original_prn_helpers.h
	$(CC) $(CFLAGS) -o $@ original_prn_helpers_test.c original_prn_helpers.c
original_log_helpers_test: original_log_helpers_test.c original_log_helpers.c original_log_helpers.h
	$(CC) $(CFLAGS) -o $@ original_log_helpers_test.c original_log_helpers.c
original_status_decoder_test: original_status_decoder_test.c original_status_decoder.c original_status_decoder.h
	$(CC) $(CFLAGS) -o $@ original_status_decoder_test.c original_status_decoder.c
original_status_gate_test: original_status_gate_test.c original_status_gate.c original_status_gate.h
	$(CC) $(CFLAGS) -o $@ original_status_gate_test.c original_status_gate.c
check: raster2printer_boundaries_recovered_test raster2printer_quality_recovered_test raster2printer_dither_recovered_test classic_read_page_test classic_write_data_test classic_process_page_modes_01_test classic_process_page_modes_23_test classic_raster_job_recovered_test original_prn_helpers_test original_log_helpers_test original_status_decoder_test original_status_gate_test
	./raster2printer_boundaries_recovered_test
	./raster2printer_quality_recovered_test
	./raster2printer_dither_recovered_test
	./classic_read_page_test
	./classic_write_data_test
	./classic_process_page_modes_01_test
	./classic_process_page_modes_23_test
	./classic_raster_job_recovered_test
	./original_status_decoder_test
	./original_status_gate_test
	./original_prn_helpers_test
	./original_log_helpers_test
clean:
	rm -f raster2printer_boundaries_recovered_test raster2printer_quality_recovered_test raster2printer_dither_recovered_test classic_read_page_test classic_write_data_test classic_process_page_modes_01_test classic_process_page_modes_23_test classic_raster_job_recovered_test classic_raster_cups_adapter original_prn_helpers_test original_log_helpers_test original_device_uri_test original_status_decoder_test original_status_gate_test

# Experimental native CUPS shell for the ordinary vendor stdout raster path.
# Original IPP remains incomplete; the recovered deferred PRN path is linked.
classic_raster_cups_adapter: classic_raster_cups_adapter.c classic_raster_job_recovered.c classic_read_page.c classic_process_page_modes_01.c classic_process_page_modes_23.c raster2printer_boundaries_recovered.c raster2printer_dither_recovered.c raster2printer_quality_recovered.c raster2printer_matrices_recovered.c original_prn_helpers.c original_prn_helpers.h
	$(CC) $(CFLAGS) -o $@ classic_raster_cups_adapter.c classic_raster_job_recovered.c classic_read_page.c classic_process_page_modes_01.c classic_process_page_modes_23.c raster2printer_boundaries_recovered.c raster2printer_dither_recovered.c raster2printer_quality_recovered.c raster2printer_matrices_recovered.c original_prn_helpers.c -lcups

# Native macOS-only, original manufacturer URI parser and socket routine.
# The test uses a usb:// URI and cannot connect to a network printer.
original_device_uri_test: original_device_uri_test.c original_device_uri.c original_device_uri.h
	$(CC) $(CFLAGS) -o $@ original_device_uri_test.c original_device_uri.c -lcups
