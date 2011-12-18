
#ifndef vlang_thread_context_h
#define vlang_thread_context_h

struct v_runtime;
struct v_heap;

typedef struct v_thread_context {
    struct v_runtime *runtime;
    struct v_heap *heap;
} v_thread_context;

typedef struct v_thread_context_ns {
    v_thread_context *(*get_current)(struct v_runtime *rt);
} v_thread_context_ns;

extern const v_thread_context v_ctx;

#endif
