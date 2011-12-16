#ifndef vlang_memory_h
#define vlang_memory_h

struct v_object;
struct v_type;
struct v_thread_context;

typedef struct {
	struct v_object (*alloc)(struct v_thread_context *ctx,
                              struct v_type *type);
} v_memory_ns;

extern const v_memory_ns v_mem;

#endif
