#include "v_array.h"
#include "v_thread_context.h"
#include "v_type.h"
#include "v_memory.h"
#include "v_object.h"
#include "v_runtime.h"
#include "v_string.h"
#include <memory.h>

static uword alignOffset(uword offset, uword on) {
    return (offset + (on - 1)) & (~(on - 1));
}

vArrayRef vArrayCreate(vThreadContextRef ctx,
                       vTypeRef elemType,
                       uword num_elements) {
    return vHeapAllocArray(ctx, v_false, elemType, num_elements);
}

pointer vArrayDataPointer(vArrayRef arr) {
    return (pointer)alignOffset((uword)arr->data, arr->alignment);
}

uword vArraySize(vArrayRef arr) {
    return arr->num_elements;
}

vArrayRef v_bootstrap_array_create(vThreadContextRef ctx,
                                   vTypeRef type,
                                   uword num_elements,
                                   uword elem_size,
                                   u8 alignment) {
    return v_bootstrap_array_alloc(ctx, type, num_elements, elem_size, alignment);
}

// There is currently no way to know if this went well or not.
// TODO: need error handling.
vObject vArrayCopy(vArrayRef from, vArrayRef to) {
    pointer a1Data, a2Data;
    
    if(from->element_type != to->element_type) {
        return NULL;
    }
    if(from->num_elements > to->num_elements) {
        return NULL;
    }
    
    a1Data = vArrayDataPointer(from);
    a2Data = vArrayDataPointer(to);
    
    memcpy(a1Data, a2Data, from->element_type->size * from->num_elements);
    return NULL;
}

void v_bootstrap_array_init_type(vThreadContextRef ctx) {
    ctx->runtime->builtInTypes.array->fields = NULL;
    ctx->runtime->builtInTypes.array->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.array->name = v_bootstrap_string_create(ctx, "Array");
    ctx->runtime->builtInTypes.array->size = sizeof(vArray);
}

