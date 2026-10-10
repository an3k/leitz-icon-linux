#ifndef LEITZ_PROBE_FORMAT_H
#define LEITZ_PROBE_FORMAT_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Formats one already captured binary reply; never accesses a printer. */
int leitz_probe_format(FILE *output, uint8_t opcode,
                       const uint8_t *bytes, size_t length);
#endif
