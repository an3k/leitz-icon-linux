CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror

.PHONY: all check clean
all: command2leitz_recovered_test classic_command_original_utils_test classic_command_write_data_test original_command_file_helpers_test original_command_main_flow_test

command2leitz_recovered_test: command2leitz_recovered_test.c command2leitz_recovered.c command2leitz_recovered.h
	$(CC) $(CFLAGS) -o $@ command2leitz_recovered_test.c command2leitz_recovered.c

classic_command_original_utils_test: classic_command_original_utils_test.c classic_command_original_utils.c classic_command_original_utils.h
	$(CC) $(CFLAGS) -o $@ classic_command_original_utils_test.c classic_command_original_utils.c

classic_command_write_data_test: classic_command_write_data_test.c classic_command_write_data.c classic_command_write_data.h
	$(CC) $(CFLAGS) -o $@ classic_command_write_data_test.c classic_command_write_data.c

original_command_file_helpers_test: original_command_file_helpers_test.c original_command_file_helpers.c original_command_file_helpers.h classic_command_original_utils.c classic_command_original_utils.h
	$(CC) $(CFLAGS) -o $@ original_command_file_helpers_test.c original_command_file_helpers.c classic_command_original_utils.c
original_command_main_flow_test: original_command_main_flow_test.c original_command_main_flow.c original_command_main_flow.h
	$(CC) $(CFLAGS) -o $@ original_command_main_flow_test.c original_command_main_flow.c
check: command2leitz_recovered_test classic_command_original_utils_test classic_command_write_data_test original_command_file_helpers_test original_command_main_flow_test
	./command2leitz_recovered_test
	./classic_command_original_utils_test
	./classic_command_write_data_test
	./original_command_file_helpers_test
	./original_command_main_flow_test

clean:
	rm -f command2leitz_recovered_test classic_command_original_utils_test classic_command_write_data_test original_command_file_helpers_test command2leitz_recovered_cups original_command_main_flow_test command2leitz_original_main_candidate

# CUPS 2.x command adapter: macOS uses its own official SDK.
# The Linux target is strictly an offline experiment with the local libcups.
ifeq ($(shell uname -s),Darwin)
CUPS_LINK = -lcups
else
CUPS_LINK = -l:libcups.so.2
endif
command2leitz_recovered_cups: command2leitz_cups.c command2leitz_recovered.c command2leitz_recovered.h
	$(CC) $(CFLAGS) -o $@ command2leitz_cups.c command2leitz_recovered.c $(CUPS_LINK)

# Separate OEM _main/CUPS candidate; the earlier command2leitz_recovered_cups
# stays unchanged. This candidate is for native manufacturer comparison only.
command2leitz_original_main_candidate: original_command_main_cups_candidate.c original_command_main_flow.c original_command_main_flow.h command2leitz_recovered.c command2leitz_recovered.h original_command_file_helpers.c original_command_file_helpers.h classic_command_original_utils.c classic_command_original_utils.h
	$(CC) $(CFLAGS) -o $@ original_command_main_cups_candidate.c original_command_main_flow.c command2leitz_recovered.c original_command_file_helpers.c classic_command_original_utils.c $(CUPS_LINK)
