#include "v_array.h"
#include "v_thread_context.h"
#include "v_type.h"
#include "v_memory.h"
#include "v_object.h"
#include "v_runtime.h"
#include "v_string.h"
#include <memory.h>

static vArrayRef create(vThreadContextRef ctx,
                       vTypeRef elemType,
                       uword num_elements) {
    vObject obj = v_mem.alloc(ctx, ctx->heap, ctx->runtime->built_in_types.array);
    vArrayRef ret = obj.value.pointer;
    uword byte_size;
    
    ret->element_type = elemType;
    ret->num_elements = num_elements;
    
    if(elemType->kind == V_T_OBJECT) {
        byte_size = sizeof(pointer) * num_elements;
    } else {
        byte_size = elemType->size * num_elements;
    }
    /* TODO: integrate with GC in v_memory
             so that the allocated space counts toward 
             the GC threshold */
    ret->data = vMalloc(byte_size);
    memset(ret->data, 0, byte_size);
    return ret;
}

static void destroy(vArrayRef arr) {
    if(arr->element_type->kind == V_T_OBJECT) {
        /* TODO: loop through objects and call destructors
                 before deallocating them. Need protocols for this */
    }
    vFree(arr->data);
}

static pointer data_pointer(vArrayRef arr) {
    return arr->data;
}

static uword size(vArrayRef arr) {
    return arr->num_elements;
}

vArrayRef v_bootstrap_array_create(vTypeRef type,
                                  uword num_elements,
                                  uword byte_size) {
    vArrayRef ret = vMalloc(sizeof(vArray));
    ret->data = vMalloc(byte_size);
    ret->element_type = type;
    ret->num_elements = num_elements;
    return ret;
}

void v_bootstrap_array_init_type(vRuntimeRef rt) {
    rt->built_in_types.array->fields = NULL;
    rt->built_in_types.array->kind = V_T_OBJECT;
    rt->built_in_types.array->name = v_bootstrap_string_create("Array");
    rt->built_in_types.array->size = sizeof(vArray);
}

const vArray_ns v_arr = {
    create,
    destroy,
    data_pointer,
    size
};

