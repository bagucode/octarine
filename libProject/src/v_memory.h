#ifndef vlang_memory_h
#define vlang_memory_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_object.h"

struct v_type;
struct v_thread_context;
struct v_runtime;

typedef struct v_heap v_heap;

typedef struct {
    v_heap *(*create_heap)(v_bool synchronized, uword gc_limit);
    void (*destroy_heap)(v_heap *heap);
	v_object (*alloc)(struct v_thread_context *ctx,
                      struct v_heap *heap,
                      struct v_type *type);
    void (*force_gc)(v_heap *heap);
} v_memory_ns;

v_object v_bootstrap_memory_alloc(v_heap *heap,
                                  struct v_type *proto_type,
                                  uword size);

extern const v_memory_ns v_mem;

#endif
