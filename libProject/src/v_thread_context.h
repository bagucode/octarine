
#ifndef vlang_thread_context_h
#define vlang_thread_context_h

#include "v_typedefs.h"
#include "../../platformProject/src/v_platform.h"

struct vThreadContext {
    vRuntimeRef runtime;
    vHeapRef heap;
    vRootSetRef roots;
};

vThreadContextRef vThreadContextGetCurrent(vRuntimeRef rt);

vThreadContextRef vThreadContextCreate(vRuntimeRef runtime,
                                       uword threadHeapInitialSize);

void vThreadContextDestroy(vThreadContextRef ctx);

extern const vThreadContext v_ctx;

#endif
