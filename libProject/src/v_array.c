#include "v_array.h"
#include "v_thread_context.h"
#include "v_type.h"
#include "v_memory.h"
#include "v_object.h"
#include "v_runtime.h"
#include "v_string.h"
#include <memory.h>

vArrayRef vArrayCreate(vThreadContextRef ctx,
                       vTypeRef elemType,
                       uword num_elements) {
    return vHeapAllocArray(ctx, v_false, elemType, num_elements);
}

pointer vArrayDataPointer(vArrayRef arr) {
    return arr->data;
}

uword vArraySize(vArrayRef arr) {
    return arr->num_elements;
}

vArrayRef v_bootstrap_array_create(vThreadContextRef ctx,
                                   vTypeRef type,
                                   uword num_elements,
                                   uword elem_size) {
    return v_bootstrap_array_alloc(ctx, type, num_elements, elem_size);
}

void v_bootstrap_array_init_type(vThreadContextRef ctx) {
    ctx->runtime->builtInTypes.array->fields = NULL;
    ctx->runtime->builtInTypes.array->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.array->name = v_bootstrap_string_create(ctx, "Array");
    ctx->runtime->builtInTypes.array->size = sizeof(vArray);
}

