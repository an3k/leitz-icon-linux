#ifndef ORIGINAL_COMMAND_NATIVE_WRITE_DATA_H
#define ORIGINAL_COMMAND_NATIVE_WRITE_DATA_H
#include <stddef.h>
#include <stdint.h>

/* The original x86_64 command2leitz _WriteData has three register
 * parameters (data, size, socket fd), followed by a large C++ descriptor
 * passed by value on the stack. The OEM accesses the descriptor at
 * stack-argument offset 0x50 for its scheme. It shares the recovered
 * classic 0x10a8 descriptor image with raster2printer.
 * This is an OEM-style C ABI reconstruction, NOT a claim that every
 * C++ descriptor field, padding, or compiler-generated code is proven. */
#define CLASSIC_COMMAND_NATIVE_DESCRIPTOR_SIZE 0x10a8u
typedef struct {
    unsigned char bytes[CLASSIC_COMMAND_NATIVE_DESCRIPTOR_SIZE];
} classic_command_native_descriptor;

int WriteData(const void *data, size_t size, int socket_fd,
              classic_command_native_descriptor descriptor);
#endif
