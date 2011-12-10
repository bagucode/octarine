
#ifndef vlang_thread_context_h
#define vlang_thread_context_h

struct v_runtime;

typedef struct v_thread_context {
    struct v_runtime *runtime;
} v_thread_context;

#endif
