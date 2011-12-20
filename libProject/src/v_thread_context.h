
#ifndef vlang_thread_context_h
#define vlang_thread_context_h

struct vRuntime;
struct vHeap;

typedef struct vThreadContext {
    struct vRuntime *runtime;
    struct vHeap *heap;
} vThreadContext;

typedef struct vThreadContext_ns {
    vThreadContext *(*get_current)(struct vRuntime *rt);
} vThreadContext_ns;

extern const vThreadContext v_ctx;

#endif
