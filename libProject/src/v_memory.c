#include "v_memory.h"
#include "v_object.h"
#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "../../platformProject/src/v_platform.h"

static v_object alloc(v_thread_context *ctx, v_type *t) {
    v_object ret;
    ret.type = t;
    
    if(v_t.is_primitive(ctx, t)) {
        ret.value.uword = 0;
    }
    else {
		ret.value.pointer = v_pf.memory.malloc(t->size);
    }
    return ret;
}

static v_object alloc_raw(v_thread_context *ctx, v_type *proto_type, uword size) {
    v_object ret;
    ret.type = proto_type;
    ret.value.pointer = v_pf.memory.malloc(size);
    return ret;
}

const v_memory_ns v_mem = {
    alloc,
    alloc_raw
};
