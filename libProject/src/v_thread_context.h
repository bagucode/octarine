
#ifndef vlang_thread_context_h
#define vlang_thread_context_h

#include "v_typedefs.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct vThreadContext {
    vRuntimeRef runtime;
    vHeapRef heap;
    vRootSetRef roots;
};

vThreadContextRef vThreadContextGetCurrent(vRuntimeRef rt);

extern const vThreadContext v_ctx;

#if defined (__cplusplus)
}
#endif

#endif
