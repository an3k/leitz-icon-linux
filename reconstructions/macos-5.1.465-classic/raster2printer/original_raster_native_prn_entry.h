#ifndef ORIGINAL_RASTER_NATIVE_PRN_ENTRY_H
#define ORIGINAL_RASTER_NATIVE_PRN_ENTRY_H

/* OEM macOS 5.1.465 raster2printer native PRN helper ABI.
 * _SendDebugFileToPrinter 0x100001b40..0x100001e0c:
 * no caller register arguments; both original exit paths converge at
 * 0x100001e05 xorl %eax,%eax and 0x100001e0c retq, so the source-level
 * callable boundary returns a 32-bit int value 0, not void.
 * Existing original helper implementation (and stdout byte loop) is
 * reused without any additional device I/O or altered retry handling.
 */
int SendDebugFileToPrinter(void);

#endif
