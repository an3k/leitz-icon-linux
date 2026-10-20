#include "monitor2_owned_cleanup.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
    leitz_lm_owned_registry registry;
    leitz_lm_owned_port *target;
    int host_closed,freed_ports,free_names,lock_depth;
    char steps[32];size_t n;
    int original_underlying;
} fixture;
static void step(fixture *f,char c) { assert(f->n+1<sizeof f->steps);f->steps[f->n++]=c;f->steps[f->n]=0; }
static int32_t no_delete(void *ctx,void *handle) { (void)ctx;(void)handle;assert(0);return -1; }
static int32_t no_event(void *ctx,void *handle) { (void)ctx;(void)handle;assert(0);return -1; }
static void lock(void *ctx) { fixture *f=ctx;step(f,'L');assert(++f->lock_depth==1); }
static void unlock(void *ctx){ fixture *f=ctx;step(f,'U');assert(--f->lock_depth==0); }
static void free_any(void *ctx,void *ptr){
    fixture *f=ctx;
    if(ptr==f->target){ step(f,'P');f->freed_ports++;f->target=NULL; }
    else { step(f,'N');f->free_names++; }
    free(ptr);
}
static int32_t host_close(void *ctx,void *handle) {
    fixture *f=ctx;assert(handle==&f->original_underlying);step(f,'C');f->host_closed++;return 0;
}
static void release_port(void *ctx) {
    fixture *f=ctx;
    const leitz_lm_owned_callbacks callbacks={no_delete,no_event,lock,unlock,free_any};
    assert(leitz_lm_release_owned_port(&f->registry,f->target,&callbacks,f)==0);
}
int main(void){
    fixture f={0};
    f.target=calloc(1,sizeof *f.target);assert(f.target);
    f.target->name_allocation=malloc(5);assert(f.target->name_allocation);
    f.registry.head=f.target;
    leitz_lm_document_port p={.context=&f,.underlying_port=&f.original_underlying,
        .close_port=host_close,.release_port=release_port};
    assert(leitz_lm_monitor2_close_port(&p)==1);
    assert(strcmp(f.steps,"CLUNP")==0);
    assert(f.host_closed==1&&f.freed_ports==1&&f.free_names==1);
    assert(f.registry.head==NULL&&f.lock_depth==0);
    puts("Windows MONITOR2 existing ClosePort -> recovered port destruction: PASS");
    return 0;
}
