#define _POSIX_C_SOURCE 200809L
#include "leitz_probe_io.h"
#include <errno.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>

static long long monotonic_ms(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) return -1;
    return (long long)t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

static int until(long long deadline) {
    const long long now = monotonic_ms();
    if (now < 0) return -1;
    if (now >= deadline) return 0;
    const long long remaining = deadline - now;
    return (remaining > 2147483647LL) ? 2147483647 : (int)remaining;
}

static int ready(int fd, short events, int timeout) {
    struct pollfd p = {.fd=fd, .events=events, .revents=0};
    int n;
    do { n = poll(&p, 1, timeout); } while (n < 0 && errno == EINTR);
    if (n <= 0) return n;
    if (p.revents & (POLLERR | POLLNVAL)) return -1;
    if (p.revents & events) return 1;
    return -1; /* HUP with no readable bytes is not a valid response. */
}

int leitz_probe_exchange(int fd, uint8_t opcode, uint8_t *buffer,
                         size_t capacity, size_t *received,
                         unsigned overall_timeout_ms, unsigned idle_gap_ms) {
    if (fd < 0 || !buffer || !received || !capacity ||
        capacity > LEITZ_PROBE_MAX_RESPONSE ||
        !overall_timeout_ms || !idle_gap_ms ||
        (opcode != 'A' && opcode != 'V' && opcode != 'P')) return -1;
    *received = 0;
    const long long now = monotonic_ms();
    if (now < 0) return LEITZ_PROBE_IO_ERROR;
    const long long deadline = now + overall_timeout_ms;
    const uint8_t command[2] = {0x1b, opcode};
    size_t sent = 0;
    while (sent < sizeof command) {
        const int wait = until(deadline);
        if (wait < 0) return LEITZ_PROBE_IO_ERROR;
        if (!wait) return LEITZ_PROBE_TIMEOUT;
        if (ready(fd, POLLOUT, wait) <= 0) return LEITZ_PROBE_IO_ERROR;
        ssize_t n = write(fd, command + sent, sizeof command - sent);
        if (n < 0 && (errno == EAGAIN || errno == EINTR)) continue;
        if (n <= 0) return LEITZ_PROBE_IO_ERROR;
        sent += (size_t)n;
    }
    long long last_data = -1;
    for (;;) {
        const long long end = (last_data >= 0 &&
                               last_data + (long long)idle_gap_ms < deadline)
                              ? last_data + (long long)idle_gap_ms : deadline;
        const int wait = until(end);
        if (wait < 0) return LEITZ_PROBE_IO_ERROR;
        if (!wait) return (last_data >= 0 &&
                             last_data + (long long)idle_gap_ms <= deadline)
                            ? 0 : LEITZ_PROBE_TIMEOUT;
        int status = ready(fd, POLLIN, wait);
        if (!status) {
            /* A timed-out poll is not proof of message completion. Only the
             * full response-idle gap, reached before the overall deadline,
             * may end an otherwise unframed snapshot. */
            const long long stopped_at = monotonic_ms();
            if (stopped_at >= 0 && last_data >= 0 &&
                last_data + (long long)idle_gap_ms <= deadline &&
                stopped_at >= last_data + (long long)idle_gap_ms) return 0;
            return LEITZ_PROBE_TIMEOUT;
        }
        if (status < 0) return LEITZ_PROBE_IO_ERROR;
        uint8_t extra;
        uint8_t *dst = *received < capacity ? buffer + *received : &extra;
        const ssize_t n = read(fd, dst, 1); /* Always detect the 1025th byte. */
        if (n < 0 && (errno == EAGAIN || errno == EINTR)) continue;
        if (n <= 0) return LEITZ_PROBE_IO_ERROR;
        if (*received >= capacity) return LEITZ_PROBE_OVERFLOW;
        ++*received;
        last_data = monotonic_ms();
        if (last_data < 0) return LEITZ_PROBE_IO_ERROR;
    }
}
