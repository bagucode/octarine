#ifndef vlang_memory_h
#define vlang_memory_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_object.h"

struct vType;
struct vThreadContext;
struct vRuntime;

typedef struct vHeap vHeap;

typedef struct {
    vHeap *(*create_heap)(v_bool synchronized, uword gc_limit);
    void (*destroy_heap)(vHeap *heap);
	vObject (*alloc)(struct vThreadContext *ctx,
                      struct vHeap *heap,
                      struct vType *type);
    void (*force_gc)(vHeap *heap);
} v_memory_ns;

vObject v_bootstrap_memory_alloc(vHeap *heap,
                                  struct vType *proto_type,
                                  uword size);

extern const v_memory_ns v_mem;

#endif
