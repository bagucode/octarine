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

oArrayRef _oArrayCreate(vThreadContextRef ctx,
                       vTypeRef elemType,
                       uword num_elements) {
    oROOTS(ctx)
    oENDROOTS
    if(ctx->error) return NULL;
	oRETURN(oHeapAllocArray(elemType, num_elements));
    oENDFN(oArrayRef)
}

pointer oArrayDataPointer(oArrayRef arr) {
    return (pointer)alignOffset((uword)arr->data, arr->alignment);
}

uword oArraySize(oArrayRef arr) {
    return arr->num_elements;
}

oArrayRef v_bootstrap_array_create(vRuntimeRef rt,
	                               oHeapRef heap,
                                   vTypeRef type,
                                   uword num_elements,
                                   uword elem_size,
                                   u8 alignment) {
    return v_bootstrap_array_alloc(rt, heap, type, num_elements, elem_size, alignment);
}

void _oArrayCopy(vThreadContextRef ctx, oArrayRef from, oArrayRef to) {
    pointer a1Data, a2Data;

    if(ctx->error) return;
    if(from->element_type != to->element_type) {
        vErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
        return;
    }
    if(from->num_elements > to->num_elements) {
        vErrorSet(ctx, ctx->runtime->builtInConstants.indexOutOfBounds);
        return;
    }
    
    a1Data = oArrayDataPointer(from);
    a2Data = oArrayDataPointer(to);
    
    memcpy(a1Data, a2Data, from->element_type->size * from->num_elements);
    return;
}

void _oArrayPut(vThreadContextRef ctx, oArrayRef arr, uword idx, pointer src, vTypeRef srcType) {
    char* data = (char*)oArrayDataPointer(arr);
    pointer* datap;

    if(ctx->error) return;
    if(vTypeIsObject(arr->element_type)
       && arr->element_type != ctx->runtime->builtInTypes.any
       && srcType != arr->element_type) {
        vErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
        return;
    }
    else if(vTypeIsStruct(srcType)
            && srcType != arr->element_type) {
        vErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
        return;
    }
    if(arr->num_elements <= idx) {
        vErrorSet(ctx, ctx->runtime->builtInConstants.indexOutOfBounds);
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

void _oArrayGet(vThreadContextRef ctx, oArrayRef arr, uword idx, pointer dest, vTypeRef destType) {
    char* data = (char*)oArrayDataPointer(arr);
    pointer *datap, *destp;
    vObject obj;

    if(ctx->error) return;
    if(arr->num_elements <= idx) {
        vErrorSet(ctx, ctx->runtime->builtInConstants.indexOutOfBounds);
        return;
    }
    
    if(vTypeIsObject(destType)) {
        datap = (pointer*)data;
        destp = (pointer*)dest;
        obj = datap[idx];
        
        if(arr->element_type == ctx->runtime->builtInTypes.any
           && destType != ctx->runtime->builtInTypes.any) {
            if(obj && vObjectGetType(ctx, obj) != destType) {
                vErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
                return;
            }
        }
        else if(destType != ctx->runtime->builtInTypes.any
                && destType != arr->element_type) {
            vErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
            return;
        }
        destp[0] = obj;
    }
    else {
        if(arr->element_type != destType) {
            vErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
            return;
        }
        data = data + destType->size * (idx + 1);
        memcpy(dest, data, destType->size);
    }
}

void v_bootstrap_array_init_type(vRuntimeRef rt, oHeapRef heap) {
    rt->builtInTypes.array->fields = NULL;
    rt->builtInTypes.array->kind = V_T_OBJECT;
    rt->builtInTypes.array->name = v_bootstrap_string_create(rt, heap, "Array");
    rt->builtInTypes.array->size = sizeof(oArray);
}

