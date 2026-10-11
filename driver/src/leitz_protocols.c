#define _POSIX_C_SOURCE 200809L
#include "leitz_protocols.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef LEITZ_DEFAULT_PROTOCOL_DIR
#define LEITZ_DEFAULT_PROTOCOL_DIR "/usr/lib/leitz/protocols"
#endif

/* The filter is a fresh process per print job: no directory watcher,
 * module manager, queue lock, or persistent module registry is needed. */
static void *modules[2];
static const leitz_protocol_ops *drivers[2];
static unsigned attempted[2];

const leitz_protocol_ops *leitz_protocol_get(leitz_protocol requested) {
  if (requested == LEITZ_PROTOCOL_AUTO) {
    const leitz_protocol_ops *preferred=leitz_protocol_get(LEITZ_PROTOCOL_CURRENT);
    return preferred ? preferred : leitz_protocol_get(LEITZ_PROTOCOL_NATIVE);
  }
  if (requested != LEITZ_PROTOCOL_CURRENT && requested != LEITZ_PROTOCOL_NATIVE)
    return NULL;
  unsigned i=(unsigned)requested;
  if (attempted[i]) return drivers[i];
  attempted[i]=1;
  const char *dir=getenv("LEITZ_PROTOCOL_DIR");
  if (!dir || !*dir) dir=LEITZ_DEFAULT_PROTOCOL_DIR;
  const char *file=i==LEITZ_PROTOCOL_CURRENT?"leitz-current.so":"leitz-native.so";
  const char *name=i==LEITZ_PROTOCOL_CURRENT?"Current":"Native";
  char path[4096];
  int n=snprintf(path,sizeof path,"%s/%s",dir,file);
  if (n<0 || (size_t)n>=sizeof path) return NULL;
  void *handle=dlopen(path,RTLD_NOW|RTLD_LOCAL);
  if (!handle) return NULL;
  const leitz_protocol_ops *ops=dlsym(handle,"leitz_protocol_driver");
  if (!ops || ops->abi_version!=LEITZ_PROTOCOL_ABI ||
      ops->struct_size!=sizeof(leitz_protocol_ops) ||
      !ops->name || strcmp(ops->name,name) ||
      !ops->state_size || !ops->begin || !ops->row ||
      !ops->end_page || !ops->finish || !ops->validate) {
    dlclose(handle);
    return NULL;
  }
  modules[i]=handle;
  drivers[i]=ops;
  return ops;
}
