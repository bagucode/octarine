#include "v_memory.h"
#include "v_object.h"
#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include <stdlib.h>

v_object alloc(v_thread_context *ctx, v_type *t) {
    v_object ret;
    ret.type = t;
    
    if(v_t.is_primitive(ctx, t)) {
        ret.value.uword = 0;
    }
    else {
        ret.value.pointer = malloc(t->size);
    }
    return ret;
}

const v_memory_ns const v_mem = {
    alloc
};
