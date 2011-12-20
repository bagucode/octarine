#ifndef vlang_memory_h
#define vlang_memory_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_object.h"
#include "v_typedefs.h"

typedef struct {
    vHeapRef (*create_heap)(v_bool synchronized, uword gc_limit);
    void (*destroy_heap)(vHeapRef heap);
	vObject (*alloc)(vThreadContextRef ctx,
                     vHeapRef heap,
                     vTypeRef type);
    void (*force_gc)(vHeapRef heap);
} v_memory_ns;

vObject v_bootstrap_memory_alloc(vHeapRef heap,
                                 vTypeRef proto_type,
                                 uword size);

extern const v_memory_ns v_mem;

#endif
