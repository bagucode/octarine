
#ifndef vlang_thread_context_h
#define vlang_thread_context_h

#include "v_typedefs.h"

struct vThreadContext {
    vRuntimeRef runtime;
    vHeapRef heap;
};

typedef struct vThreadContext_ns {
    vThreadContextRef (*get_current)(vRuntimeRef rt);
} vThreadContext_ns;

extern const vThreadContext v_ctx;

#endif
