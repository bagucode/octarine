#include "v_array.h"
#include "v_thread_context.h"
#include "v_type.h"
#include "v_memory.h"
#include "v_object.h"
#include "v_runtime.h"
#include "v_string.h"
#include "v_error.h"
#include <memory.h>

static uword alignOffset(uword offset, uword on) {
    return (offset + (on - 1)) & (~(on - 1));
}

vArrayRef vArrayCreate(vThreadContextRef ctx,
                       vTypeRef elemType,
                       uword num_elements) {
	return vHeapAllocArray(ctx->runtime, ctx->heap, elemType, num_elements);
}

pointer vArrayDataPointer(vArrayRef arr) {
    return (pointer)alignOffset((uword)arr->data, arr->alignment);
}

uword vArraySize(vArrayRef arr) {
    return arr->num_elements;
}

vArrayRef v_bootstrap_array_create(vRuntimeRef rt,
	                               vHeapRef heap,
                                   vTypeRef type,
                                   uword num_elements,
                                   uword elem_size,
                                   u8 alignment) {
    return v_bootstrap_array_alloc(rt, heap, type, num_elements, elem_size, alignment);
}

void vArrayCopy(vThreadContextRef ctx, vArrayRef from, vArrayRef to) {
    pointer a1Data, a2Data;
    
    if(from->element_type != to->element_type) {
        vErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
        return;
    }
    if(from->num_elements > to->num_elements) {
        vErrorSet(ctx, ctx->runtime->builtInConstants.arrayIndexOutOfBounds);
        return;
    }
    
    a1Data = vArrayDataPointer(from);
    a2Data = vArrayDataPointer(to);
    
    memcpy(a1Data, a2Data, from->element_type->size * from->num_elements);
    return;
}

void vArrayPut(vThreadContextRef ctx, vArrayRef arr, uword idx, pointer src, vTypeRef srcType) {
    char* data = (char*)vArrayDataPointer(arr);
    pointer* datap;

    if(vTypeIsObject(arr->element_type)
       && arr->element_type != ctx->runtime->builtInTypes.any
       && srcType != arr->element_type) {
        // TODO: ERROR
        return;
    }
    else if(vTypeIsStruct(srcType)
            && srcType != arr->element_type) {
        // TODO: ERROR
        return;
    }
    if(arr->num_elements <= idx) {
        // TODO: ERROR
        return;
    }
    
    if(vTypeIsObject(srcType)) {
        datap = (pointer*)data;
        datap[idx] = src;
    }
    else {
        data = data + srcType->size * (idx + 1);
        memcpy(data, src, srcType->size);
    }
}

void vArrayGet(vThreadContextRef ctx, vArrayRef arr, uword idx, pointer dest, vTypeRef destType) {
    char* data = (char*)vArrayDataPointer(arr);
    pointer *datap, *destp;
    vObject obj;

    if(arr->num_elements <= idx) {
        // TODO: ERROR
        return;
    }
    
    if(vTypeIsObject(destType)) {
        datap = (pointer*)data;
        destp = (pointer*)dest;
        obj = datap[idx];
        
        if(arr->element_type == ctx->runtime->builtInTypes.any
           && destType != ctx->runtime->builtInTypes.any) {
            if(obj && vObjectGetType(ctx, obj) != destType) {
                // TODO: ERROR
                return;
            }
        }
        else if(destType != ctx->runtime->builtInTypes.any
                && destType != arr->element_type) {
            // TODO: ERROR
            return;
        }
        destp[0] = obj;
    }
    else {
        if(arr->element_type != destType) {
            // TODO: ERROR
            return;
        }
        data = data + destType->size * (idx + 1);
        memcpy(dest, data, destType->size);
    }
}

void v_bootstrap_array_init_type(vRuntimeRef rt, vHeapRef heap) {
    rt->builtInTypes.array->fields = NULL;
    rt->builtInTypes.array->kind = V_T_OBJECT;
    rt->builtInTypes.array->name = v_bootstrap_string_create(rt, heap, "Array");
    rt->builtInTypes.array->size = sizeof(vArray);
}

