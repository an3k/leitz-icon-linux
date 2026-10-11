#include "leitz_protocol.h"

/* GCC/ELF optional symbols. Deleting either source file removes its entire
 * implementation from the linked binaries. No weak fallback implementation. */
extern const leitz_protocol_ops leitz_current_driver __attribute__((weak));
extern const leitz_protocol_ops leitz_native_driver __attribute__((weak));

const leitz_protocol_ops *leitz_protocol_get(leitz_protocol protocol) {
  switch(protocol) {
    case LEITZ_PROTOCOL_CURRENT:
      return &leitz_current_driver ? &leitz_current_driver : NULL;
    case LEITZ_PROTOCOL_NATIVE_LINUX_103:
      return &leitz_native_driver ? &leitz_native_driver : NULL;
  }
  return NULL;
}
