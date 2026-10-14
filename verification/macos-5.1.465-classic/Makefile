CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror
OEM_SRC = ../../reconstructions/macos-5.1.465-classic
.PHONY: check_write_data clean_write_data
original_write_data_semantics_test: original_write_data_semantics_test.c $(OEM_SRC)/command2leitz/classic_command_write_data.c $(OEM_SRC)/raster2printer/classic_write_data.c
	$(CC) $(CFLAGS) -o $@ original_write_data_semantics_test.c $(OEM_SRC)/command2leitz/classic_command_write_data.c $(OEM_SRC)/raster2printer/classic_write_data.c
check_write_data: original_write_data_semantics_test
	./original_write_data_semantics_test
clean_write_data:
	rm -f original_write_data_semantics_test
