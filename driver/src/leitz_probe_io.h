#ifndef LEITZ_PROBE_IO_H
#define LEITZ_PROBE_IO_H
#include <stddef.h>
#include <stdint.h>
#define LEITZ_PROBE_MAX_RESPONSE 1024U
#define LEITZ_PROBE_TIMEOUT (-2)
#define LEITZ_PROBE_IO_ERROR (-3)
#define LEITZ_PROBE_OVERFLOW (-4)
/* Byte-oriented, deadline-limited exchange on an already opened nonblocking fd.
 * Exactly one 2-byte status query per invocation. No automated retry. */
int leitz_probe_exchange(int fd, uint8_t opcode, uint8_t *buffer,
                         size_t capacity, size_t *received,
                         unsigned overall_timeout_ms, unsigned idle_gap_ms);
#endif
