
#ifndef vlang_thread_context_h
#define vlang_thread_context_h

#include "v_typedefs.h"

struct vThreadContext {
    vRuntimeRef runtime;
    vHeapRef heap;
    vRootSetRef roots;
};

vThreadContextRef vThreadContextGetCurrent(vRuntimeRef rt);

extern const vThreadContext v_ctx;

#endif
